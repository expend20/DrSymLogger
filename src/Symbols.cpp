#include "Symbols.h"
#include <stdio.h>

void IsDrSymbolsSetOrAbort()
{
    char val[MAX_PATH];

    auto r = GetEnvironmentVariable("_NT_SYMBOL_PATH", val, sizeof(val));

    if (!r || r >= sizeof(val))
    {
        dr_printf("Get't get env var _NT_SYMBOL_PATH, is it set?\n");
        dr_abort();
    };

    if (strstr(val, "srv*"))
    {
        dr_printf(
            "srv* substring is found in _NT_SYMBOL_PATH, it won't work "
            "under DR, more info: https://dynamorio.org/page_drsyms.html\n");
        dr_abort();
    }

    if (strstr(val, "http"))
    {
        dr_printf(
            "http substring is found in _NT_SYMBOL_PATH, it won't work "
            "under DR, more info: https://dynamorio.org/page_drsyms.html\n");
        dr_abort();
    }
}

drsym_error_t GetSymbolByName(const char *full_path, const char *name,
                              size_t *offset, size_t *addr)
{

    static uint demangleFlags =
        0; // DRSYM_DEFAULT_FLAGS;
           //(DRSYM_DEMANGLE | DRSYM_DEMANGLE_PDB_TEMPLATES);

    size_t off;

    drsym_error_t r = drsym_lookup_symbol(full_path, name, &off, demangleFlags);

    dr_printf("\"%s\" symbol lookup resulted %d\n", name, r);

    if (offset)
    {
        *offset = off;
    }
    if (addr)
    {
        module_data_t *mod = dr_lookup_module_by_name(full_path);
        if (!mod)
        {
            dr_printf("[!] Can't look up the module %s\n", full_path);
        }
        else
        {
            dr_printf("Symbol resolved successfully %p -> %s\n", offset, name);

            *addr = off + (size_t)mod->handle;
        }
    }

    return r;
}

drsym_error_t GetSymbolByAddr(const char *full_path, size_t tagOffset,
                              drsym_info_t *sym, char *name, size_t nameSize)
{

    static uint demangleFlags =
        0; // DRSYM_DEFAULT_FLAGS;
           //(DRSYM_DEMANGLE | DRSYM_DEMANGLE_PDB_TEMPLATES);

    sym->struct_size = sizeof(*sym);
    sym->name = name;
    sym->name_size = nameSize;
    // sym->file = file;
    // sym->file_size = MAXIMUM_PATH;

    drsym_error_t r =
        drsym_lookup_address(full_path, tagOffset, sym, demangleFlags);

    if (sym->name_available_size >= sym->name_size)
        dr_printf("WARNING: function name longer than max: %s\n", sym->name);

    // printf("Sym: %s | t %x | %x | n (%d) %s %s | f (%d) %s %s | "
    //         "s/e %x(%x) %x\n",
    //         data->full_path,
    //         sym.debug_kind,
    //         r,
    //         sym.name_available_size,
    //         name,
    //         sym.name,
    //         sym.file_available_size,
    //         file,
    //         sym.file,
    //         sym.start_offs,
    //         tagOffset,
    //         sym.end_offs);

    return r;
}
