#pragma once

enum class_attr {
    class_none,
};
enum method_attr {
    method_none      = 1 << 0,
    method_static    = 1 << 1
};