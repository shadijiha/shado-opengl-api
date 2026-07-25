#include "SceneUtils.h"

#if defined(SHADO_PLATFORM_MACOS)

#import <Cocoa/Cocoa.h>

// macOS native implementation of the file/alert dialogs using AppKit.
// GLFW initializes the shared NSApplication instance on macOS, so these modal
// panels can run directly. They must be invoked from the main thread (which is
// where the editor UI callbacks run).

namespace Shado {

    std::string FileDialogs::openFile(const char* filter) {
        (void)filter; // Win32-style filter strings are not applied on macOS.
        @autoreleasepool {
            NSOpenPanel* panel = [NSOpenPanel openPanel];
            [panel setCanChooseFiles:YES];
            [panel setCanChooseDirectories:NO];
            [panel setAllowsMultipleSelection:NO];

            if ([panel runModal] == NSModalResponseOK) {
                NSURL* url = [[panel URLs] firstObject];
                if (url != nil)
                    return std::string([[url path] UTF8String]);
            }
            return std::string();
        }
    }

    std::string FileDialogs::saveFile(const char* filter) {
        (void)filter;
        @autoreleasepool {
            NSSavePanel* panel = [NSSavePanel savePanel];
            [panel setCanCreateDirectories:YES];

            if ([panel runModal] == NSModalResponseOK) {
                NSURL* url = [panel URL];
                if (url != nil)
                    return std::string([[url path] UTF8String]);
            }
            return std::string();
        }
    }

    std::string FileDialogs::chooseFolder() {
        @autoreleasepool {
            NSOpenPanel* panel = [NSOpenPanel openPanel];
            [panel setCanChooseFiles:NO];
            [panel setCanChooseDirectories:YES];
            [panel setAllowsMultipleSelection:NO];
            [panel setTitle:@"Select Your Source Directory"];

            if ([panel runModal] == NSModalResponseOK) {
                NSURL* url = [[panel URLs] firstObject];
                if (url != nil)
                    return std::string([[url path] UTF8String]);
            }
            return std::string();
        }
    }

    void Dialog::alert(const std::string& message, const std::string& alertTitle, DialogIcon icon) {
        @autoreleasepool {
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setMessageText:[NSString stringWithUTF8String:alertTitle.c_str()]];
            [alert setInformativeText:[NSString stringWithUTF8String:message.c_str()]];

            switch (icon) {
            case DialogIcon::STOP:
            case DialogIcon::ERROR_ICON:
            case DialogIcon::HAND:
                [alert setAlertStyle:NSAlertStyleCritical];
                break;
            case DialogIcon::INFORMATION:
            case DialogIcon::ASTERISK:
            case DialogIcon::QUESTION:
                [alert setAlertStyle:NSAlertStyleInformational];
                break;
            default:
                [alert setAlertStyle:NSAlertStyleWarning];
                break;
            }

            [alert addButtonWithTitle:@"OK"];
            [alert runModal];
        }
    }

    void Dialog::openPathInExplorer(const std::filesystem::path& path) {
        @autoreleasepool {
            NSString* nsPath = [NSString stringWithUTF8String:path.string().c_str()];
            [[NSWorkspace sharedWorkspace] selectFile:nsPath inFileViewerRootedAtPath:@""];
        }
    }
}

#endif
