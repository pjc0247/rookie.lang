#pragma once

enum class_attr {
    class_none,

    class_systype    = 1 << 16
};
enum method_attr {
    method_none      = 1 << 0,
    method_static    = 1 << 1,

    method_syscall   = 1 << 16
};