#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#include <dr_api.h>
#include <drsyms.h>

drsym_error_t GetSymbolByName(const char *full_path, const char *name,
                              size_t *offset, size_t *addr);

drsym_error_t GetSymbolByAddr(const char *full_path, size_t tagOffset,
                              drsym_info_t *sym, char *name, size_t nameSize);

void IsDrSymbolsSetOrAbort();

#endif // _SYMBOLS_H_
