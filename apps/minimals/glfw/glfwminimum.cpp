/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <modules/opengl/inviwoopengl.h>
#include <modules/glfw/canvasglfw.h>
#include <modules/glfw/filewatcher.h>

#include <inviwo/core/common/defaulttohighperformancegpu.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/moduleregistration.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/networkdebugobserver.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace inviwo;

int main(int argc, char** argv) {
    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);
    auto consoleLogger = std::make_shared<inviwo::ConsoleLogger>();
    logger.registerLogger(consoleLogger);

    InviwoApplication inviwoApp(argc, argv, "Inviwo-GLFW");
    inviwoApp.printApplicationInfo();
    inviwoApp.setPostEnqueueFront([]() { glfwPostEmptyEvent(); });
    inviwoApp.setProgressCallback([](std::string m) {
        LogCentral::getPtr()->log("InviwoApplication", LogLevel::Info, LogAudience::User, "", "", 0,
                                  m);
    });

    inviwoApp.setFileSystemObserver(std::make_unique<inviwo::FileWatcher>(&inviwoApp));

    // Initialize all modules
    inviwoApp.registerModules(inviwo::getModuleList());

    auto& cmdparser = inviwoApp.getCommandLineParser();
    TCLAP::ValueArg<std::string> snapshotArg(
        "s", "snapshot",
        "Specify default name of each snapshot, or empty string for processor name.", false, "",
        "Snapshot default name: UPN=Use Processor name.");

    cmdparser.add(
        &snapshotArg,
        [&]() {
            std::string path = cmdparser.getOutputPath();
            if (path.empty()) path = inviwoApp.getPath(PathType::Images);
            util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path, snapshotArg.getValue());
        },
        1000);

    TCLAP::SwitchArg debugProcess("d", "debug",
                                  "Add debug logging for processor evaluation to the log");

    NetworkDebugObserver obs;
    cmdparser.add(
        &debugProcess,
        [&]() {
            inviwoApp.getProcessorNetwork()->addObserver(&obs);
            inviwoApp.getProcessorNetworkEvaluator()->addObserver(&obs);
            inviwoApp.getProcessorNetwork()->forEachProcessor(
                [&](auto* p) { p->ProcessorObservable::addObserver(&obs); });
        },
        200);

    TCLAP::SwitchArg fullscreenArg("f", "fullscreen", "Specify fullscreen if only one canvas");

    cmdparser.add(&fullscreenArg, [&]() {
        auto network = inviwoApp.getProcessorNetwork();

        std::vector<ProcessorWidget*> widgets;
        network->forEachProcessor([&](Processor* p) {
            if (p->isSink()) {
                if (auto widget = p->getProcessorWidget()) {
                    if (widget->isVisible()) {
                        widgets.push_back(widget);
                    }
                }
            }
        });

        if (widgets.size() == 1) {
            widgets[0]->setFullScreen(true);
        }
    });

    // Do this after registerModules if some arguments were added
    cmdparser.parse(inviwo::CommandLineParser::Mode::Normal);

    // Load simple scene
    inviwoApp.getProcessorNetwork()->lock();
    const std::string workspace = cmdparser.getLoadWorkspaceFromArg()
                                      ? cmdparser.getWorkspacePath()
                                      : inviwoApp.getPath(PathType::Workspaces, "/boron.inv");

    try {
        if (!workspace.empty()) {
            inviwoApp.getWorkspaceManager()->load(workspace, [&](ExceptionContext ec) {
                try {
                    throw;
                } catch (const IgnoreException& e) {
                    util::log(
                        e.getContext(),
                        "Incomplete network loading " + workspace + " due to " + e.getMessage(),
                        LogLevel::Error);
                }
            });
        }
    } catch (const AbortException& exception) {
        util::log(exception.getContext(),
                  "Unable to load network " + workspace + " due to " + exception.getMessage(),
                  LogLevel::Error);
        return 1;
    } catch (const IgnoreException& exception) {
        util::log(exception.getContext(),
                  "Incomplete network loading " + workspace + " due to " + exception.getMessage(),
                  LogLevel::Error);
        return 1;
    }

    inviwoApp.getProcessorNetwork()->unlock();

    cmdparser.processCallbacks();  // run any command line callbacks from modules.

    if (cmdparser.getQuitApplicationAfterStartup()) {
        glfwTerminate();
        return 0;
    }

    while (CanvasGLFW::getVisibleWindowCount() > 0) {
        glfwWaitEvents();
        inviwoApp.processFront();
    }

    return 0;
}
