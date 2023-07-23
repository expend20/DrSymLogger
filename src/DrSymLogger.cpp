#include "Symbols.h"
#include "args.h"

#include "dr_api.h"
#include "drmgr.h"
#include "drsyms.h"
#include "drwrap.h"

#include <map>
#include <string>
#include <vector>

static void *g_traceMut = 0;

static bool g_printSymsExec = false;
static bool g_printSymsExecConsole = false;
static bool g_printSymsInst = false;

static std::list<char *> g_moduleNames;
static std::list<char *> g_moduleSkipNames;
static std::list<char *> g_printSymsGrep;

static std::map<size_t, size_t> g_spacesByThread;
// Absolute address of func -> function name
static std::map<size_t, std::string> g_funcSyms;

static file_t g_logFile = INVALID_FILE;
static size_t g_lineNum = 1;

static void TraceSymsWrapPre(void *wrapctx, OUT void **userData)
{
    auto funcAddr = (size_t)drwrap_get_func(wrapctx);
    auto mod = dr_lookup_module((byte *)funcAddr);

    dr_mutex_lock(g_traceMut);

    auto currThread = GetCurrentThreadId();
    size_t spaces = g_spacesByThread[currThread];
    spaces++;
    g_spacesByThread[currThread] = spaces;

    std::string padding;
    padding.resize((spaces - 1) * 4);
    std::fill(padding.begin(), padding.end(), ' ');

    if (g_printSymsExecConsole)
    {
        dr_printf("%10d T:%6d [%20s]: %s -> %s\n", g_lineNum++, currThread,
                  mod ? mod->names.file_name : "<UNK>", padding.c_str(),
                  g_funcSyms[funcAddr].c_str());
    }
    else
    {
        dr_fprintf(g_logFile, "%10d T:%6d [%20s]: %s -> %s\t\n", g_lineNum++,
                   currThread, mod ? mod->names.file_name : "<UNK>",
                   padding.c_str(), g_funcSyms[funcAddr].c_str());
    }

    // Custom handlers
    if (!strcmp(g_funcSyms[funcAddr].c_str(), "LoadLibraryExW"))
    {
        dr_fprintf(g_logFile, "LoadLibraryExW: %S\n",
                   drwrap_get_arg(wrapctx, 0));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "GetModuleHandleW"))
    {
        dr_fprintf(g_logFile, "GetModuleHandleW: %S\n",
                   drwrap_get_arg(wrapctx, 0));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "strrchr"))
    {
        dr_fprintf(g_logFile, "strrchr: %s %c\n", drwrap_get_arg(wrapctx, 0),
                   drwrap_get_arg(wrapctx, 1));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "GetProcAddress"))
    {
        dr_fprintf(g_logFile, "GetProcAddress: %p %s\n",
                   drwrap_get_arg(wrapctx, 0), drwrap_get_arg(wrapctx, 1));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "Sleep"))
    {
        dr_fprintf(g_logFile, "Sleep: %d\n", drwrap_get_arg(wrapctx, 0));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "VirtuallAlloc"))
    {
        dr_fprintf(g_logFile, "VirtuallAlloc: %p %x %x %x\n",
                   drwrap_get_arg(wrapctx, 0), drwrap_get_arg(wrapctx, 1),
                   drwrap_get_arg(wrapctx, 2), drwrap_get_arg(wrapctx, 3));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "WriteProcessMemory"))
    {
        dr_fprintf(g_logFile, "WriteProcessMemory: %p %p %p %x\n",
                   drwrap_get_arg(wrapctx, 0), drwrap_get_arg(wrapctx, 1),
                   drwrap_get_arg(wrapctx, 2), drwrap_get_arg(wrapctx, 3));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "VirtualProtect"))
    {
        dr_fprintf(g_logFile, "VirtualProtect: %p %x %x %p\n",
                   drwrap_get_arg(wrapctx, 0), drwrap_get_arg(wrapctx, 1),
                   drwrap_get_arg(wrapctx, 2), drwrap_get_arg(wrapctx, 3));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "wcsicmp") ||
             !strcmp(g_funcSyms[funcAddr].c_str(), "wcscmp"))
    {

        dr_fprintf(g_logFile, "wcs[i]cmp: %S == %S\n",
                   drwrap_get_arg(wrapctx, 0), drwrap_get_arg(wrapctx, 1));

        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "strcmp") ||
             !strcmp(g_funcSyms[funcAddr].c_str(), "stricmp"))
    {

        dr_fprintf(g_logFile, "str[i]cmp: %s == %s\n",
                   drwrap_get_arg(wrapctx, 0), drwrap_get_arg(wrapctx, 1));
        goto done;
    }
    else if (!strcmp(g_funcSyms[funcAddr].c_str(), "memcmp"))
    {

        auto sz = (size_t)drwrap_get_arg(wrapctx, 2);
        auto ptr1 = (uint8_t *)drwrap_get_arg(wrapctx, 0);
        auto ptr2 = (uint8_t *)drwrap_get_arg(wrapctx, 1);

        dr_fprintf(g_logFile, "memcmp %d: ", sz);
        for (size_t i = 0; i < sz; i++)
        {
            dr_fprintf(g_logFile, "%02x ", ptr1[i]);
        }

        dr_fprintf(g_logFile, " | ");
        for (size_t i = 0; i < sz; i++)
        {
            dr_fprintf(g_logFile, "%02x ", ptr2[i]);
        }
        dr_fprintf(g_logFile, "\n");
    }
    else if (strstr(g_funcSyms[funcAddr].c_str(), "rand"))
    {
        dr_fprintf(g_logFile, "*** rand-containing function detected ***\n");
    }
    else if (strstr(g_funcSyms[funcAddr].c_str(), "cmp"))
    {
        dr_fprintf(g_logFile, "*** cmp-containing function detected ***\n");
    }

done:
    dr_mutex_unlock(g_traceMut);
}

static void TraceSymsWrapPost(void *wrapctx, void *userData)
{
    auto funcAddr = (size_t)drwrap_get_func(wrapctx);
    auto mod = dr_lookup_module((byte *)funcAddr);

    dr_mutex_lock(g_traceMut);

    auto currThread = GetCurrentThreadId();
    size_t spaces = g_spacesByThread[currThread];
    spaces--;
    g_spacesByThread[currThread] = spaces;

    std::string padding;
    padding.resize((spaces)*4);
    std::fill(padding.begin(), padding.end(), ' ');

    auto mc = drwrap_get_mcontext(wrapctx);
    if (g_printSymsExecConsole)
    {
        dr_printf("%10d T:%6d [%20s]: %s <- %s (%p)\n", g_lineNum++, currThread,
                  mod ? mod->names.file_name : "<UNK>", padding.c_str(),
                  g_funcSyms[funcAddr].c_str(), mc ? mc->xax : 0x1337beef);
    }
    else
    {
        dr_fprintf(g_logFile, "%10d T:%6d [%20s]: %s <- %s (%p)\n", g_lineNum++,
                   currThread, mod ? mod->names.file_name : "<UNK>",
                   padding.c_str(), g_funcSyms[funcAddr].c_str(),
                   mc ? mc->xax : 0x1337beef);
    }

    dr_mutex_unlock(g_traceMut);
}

static dr_emit_flags_t CodeRefsBBInstClb(void *drcontext, void *tag,
                                         instrlist_t *bb, instr_t *inst,
                                         bool for_trace, bool translating,
                                         void *user_data)
{

    module_data_t *mod = 0;
    drsym_info_t sym = {0};
    static char name[2048] = {0};
    drsym_error_t r = DRSYM_ERROR;
    size_t tagOffset = 0;

    if (!drmgr_is_first_instr(drcontext, inst))
    {
        return DR_EMIT_DEFAULT;
    }

    if (instr_get_app_pc(inst) == NULL || !instr_is_app(inst))
    {
        return DR_EMIT_DEFAULT;
    }

    mod = dr_lookup_module((byte *)tag);
    if (mod == NULL)
    {
        dr_printf("Can't get module %p\n", tag);
        return DR_EMIT_DEFAULT;
    }
    if (mod->names.file_name == NULL)
    {
        return DR_EMIT_DEFAULT;
    }

    if (g_moduleNames.size())
    {
        bool isFound = false;
        for (auto &m : g_moduleNames)
        {
            if (!_stricmp(mod->names.file_name, m))
            {
                isFound = true;
                break;
            }
        }
        if (!isFound)
        {
            return DR_EMIT_DEFAULT;
        }
    }

    for (auto &m : g_moduleSkipNames)
    {
        if (!_stricmp(mod->names.file_name, m))
        {
            return DR_EMIT_DEFAULT;
        }
    }

    tagOffset = (size_t)tag - (size_t)mod->start;
    r = GetSymbolByAddr(mod->full_path, tagOffset, &sym, name, sizeof(name));
    if (r != DRSYM_SUCCESS && r != DRSYM_ERROR_LINE_NOT_AVAILABLE)
    {
        // it always returns this
        // dr_printf("Can't lookup address %s :: %p:%x %x, dbg kind %x\n",
        //        mod->full_path,
        //        tag,
        //        tagOffset,
        //        r,
        //        sym.debug_kind);
        return DR_EMIT_DEFAULT;
    }
    // if the start of the symbol equals to the requestes offset
    if ((size_t)sym.start_offs != tagOffset)
    {
        return DR_EMIT_DEFAULT;
    }

    if (g_printSymsGrep.size())
    {
        bool matched = false;

        for (auto &el : g_printSymsGrep)
        {
            if (strstr(name, el))
            {
                matched = true;
                break;
            }
        }

        if (!matched)
        {
            return DR_EMIT_DEFAULT;
        }
    }

    if (g_printSymsInst)
    {
        dr_printf("INST SYM: %s :: %s\n", mod->names.file_name, name);
    }

    if (g_printSymsExec)
    {
        g_funcSyms[(size_t)tag] = name;

        if (!drwrap_wrap_ex((app_pc)tag, TraceSymsWrapPre, TraceSymsWrapPost,
                            tag, 0))
        {
            return DR_EMIT_DEFAULT;
        }
    }

    return DR_EMIT_DEFAULT;
}

void TraceSymsExitClb(void)
{
    if (g_logFile != INVALID_FILE)
        dr_close_file(g_logFile);
    g_logFile = INVALID_FILE;
}

static void TraceSymsInitOptions(int argc, char *argv[])
{
    g_printSymsExec = GetBinaryOption("--printSymsExec", argc, argv, false);
    g_printSymsInst = GetBinaryOption("--printSymsInst", argc, argv, false);
    g_printSymsExecConsole =
        GetBinaryOption("--printSymsExecConsole", argc, argv, false);

    GetOptionAll("--printSymsModule", argc, argv, &g_moduleNames);
    for (auto &e : g_moduleNames)
    {
        dr_printf("Trace symbols for module: %s\n", e);
    }

    GetOptionAll("--printSymsSkipModule", argc, argv, &g_moduleSkipNames);
    for (auto &e : g_moduleSkipNames)
    {
        dr_printf("Skipping trace symbols for module: %s\n", e);
    }

    GetOptionAll("--printSymsGrep", argc, argv, &g_printSymsGrep);
    for (auto &e : g_printSymsGrep)
    {
        dr_printf("Grep symbol: %s\n", e);
    }
}

void TraceSymsInit(int argc, char **argv)
{
    TraceSymsInitOptions(argc, argv);

    IsDrSymbolsSetOrAbort();

    if (g_printSymsExec && !g_printSymsExecConsole)
    {
        char path[MAX_PATH] = {0};
        dr_snprintf(path, sizeof(path), "sym_rt_trace_%d.txt",
                    GetCurrentProcessId());
        g_logFile = dr_open_file(path, DR_FILE_WRITE_OVERWRITE);
        if (g_logFile == INVALID_FILE)
        {
            dr_printf("[!] Can't crate file");
        }
        dr_printf("Writing symbols execution trace to %s...\n", path);
    }

    g_traceMut = dr_mutex_create();

    if (!drmgr_register_bb_instrumentation_event(NULL, CodeRefsBBInstClb, NULL))
    {
        DR_ASSERT(false);
    }

    dr_register_exit_event(TraceSymsExitClb);
}

DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    dr_set_client_name("DynamoRIO custom tool", "http://dynamorio.org/issues");

    drmgr_init();
    drwrap_init();

    if (drsym_init(0) != DRSYM_SUCCESS)
    {
        dr_printf("Can't load drsym");
    }

    if (dr_is_notify_on())
    {
        dr_enable_console_printing();
    }

    TraceSymsInit(argc, (char **)argv);
}
