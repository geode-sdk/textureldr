#import <Foundation/Foundation.h>

#include "VanillaResourceResolver.hpp"

std::string resolveVanillaResource(std::string_view path) {
    auto filename = [[NSString alloc] initWithBytes:path.data()
                                             length:path.size()
                                           encoding:NSUTF8StringEncoding];
    if (!filename) {
        return {};
    }

    auto directory = [filename stringByDeletingLastPathComponent];
    auto resource = [filename lastPathComponent];
    auto fullPath = [[NSBundle mainBundle] pathForResource:resource
                                                     ofType:nil
                                                inDirectory:directory.length ? directory : nil];
    return fullPath ? std::string(fullPath.UTF8String) : std::string();
}
