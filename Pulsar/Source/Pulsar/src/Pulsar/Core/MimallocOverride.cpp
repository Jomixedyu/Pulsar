// MimallocOverride.cpp
// Include this in exactly ONE translation unit to override global new/delete.
// Must be linked into the final executable so the operator new/delete symbols
// are resolved before the CRT defaults.
//
// mimalloc will also override malloc/free (MI_OVERRIDE=ON).

#include <mimalloc-new-delete.h>
