Using DynamoRIO to capture function symbol calls at runtime.

[![DrSymLogger: DBI technique](https://yt-embed.herokuapp.com/embed?v=sIc7zQgbn0Y)](https://www.youtube.com/watch?v=sIc7zQgbn0Y "DrSymLogger: DBI technique")

Fragment of capturing logs for cmd.exe:
```
-> ParseS0
    -> BinaryOperator
        -> ParseS1
            -> BinaryOperator
                -> ParseS2
                    -> BinaryOperator
                        -> ParseS3
                            -> BinaryOperator
                                -> ParseS4
                                    -> ParseRedir
                                    <- ParseRedir (0x0000000000000000)
                                    -> ParseS5
                                        -> ParseCmd
                                            -> LoadNodeTC
                                            <- LoadNodeTC (0x0000013275a2ffc0)
                                            -> Lex
                                                -> _intrinsic_setjmp
                                                <- _intrinsic_setjmp (0x0000000000000000)
                                            <- Lex (0x0000000000004000)
                                            -> mkstr
                                            <- mkstr (0x0000013275a2be30)
                                            -> Lex
                                                -> _intrinsic_setjmp
                                                <- _intrinsic_setjmp (0x0000000000000000)
                                            <- Lex (0x000000000000000a)
                                            -> ParseRedir
                                            <- ParseRedir (0x0000000000000000)
                                            -> Lex
                                                -> _intrinsic_setjmp
                                                <- _intrinsic_setjmp (0x0000000000000000)
                                            <- Lex (0x0000000000000000)
                                        <- ParseCmd (0x0000013275a2ffc0)
                                    <- ParseS5 (0x0000013275a2ffc0)
                                    -> GeToken
```

# Build

```
cmake -B build -DDynamoRIO_DIR="...\dr9\cmake"
cmake --build bulid --config RelWithDebInfo
```

# Usage

- Build the DrSymLogger.dll
- Make sure you have symbols downloaded for specified modules
  - set _NT_SYMBOL_PATH system wide for something like `srv*c:\symbols*https://msdl.microsoft.com/download/symbols`
  - Run the process under windbg, to download symbols
- Spawn a new terminal for drrun
  - set _NT_SYMBOL_PATH for specific console to a directory `c:\symbols` or where ever you have your symbols. DR has a bit different _NT_SYMBOL_PATH meaning than windbg, it does not allow anything except local directores.
- finally run the tool `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsExec --printSymsModule cmd.exe -- cmd.exe /c "echo 123; exit"`, it'll create `sym_rt_trace_*.txt` file

# Some hints

To output result to console, add `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsExec --printSymsExecConsole --printSymsModule cmd.exe -- cmd.exe /c "echo 123; exit"`. If your program outputs to console you now have synchronized trace with your logs.

You could track which symbols get instrumented once with `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsInst --printSymsModule cmd.exe -- cmd.exe /c "echo 123; exit"`, this is super fast since does not really affect runtime stage at all.

You could use simple grep feature `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsExec --printSymsExecConsole --printSymsModule cmd.exe --printSymsGrep Parse -- cmd.exe /c "echo 123; exit"`

You cold instrument literally every symbol with anything you want, look at some examples [here](https://github.com/expend20/DrSymLogger/blob/37bc4feb8f5583a91deba45dc60990177c3908c2/src/DrSymLogger.cpp#L59).



