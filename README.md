Using DynamoRIO to capture symbols at runtime.

Fragment of capturing logs for cmd.exe:
```
      1569 T:  3956 [             cmd.exe]:                  -> ParseS0
      1570 T:  3956 [             cmd.exe]:                      -> BinaryOperator
      1571 T:  3956 [             cmd.exe]:                          -> ParseS1
      1572 T:  3956 [             cmd.exe]:                              -> BinaryOperator
      1573 T:  3956 [             cmd.exe]:                                  -> ParseS2
      1574 T:  3956 [             cmd.exe]:                                      -> BinaryOperator
      1575 T:  3956 [             cmd.exe]:                                          -> ParseS3
      1576 T:  3956 [             cmd.exe]:                                              -> BinaryOperator
      1577 T:  3956 [             cmd.exe]:                                                  -> ParseS4
      1578 T:  3956 [             cmd.exe]:                                                      -> ParseRedir
      1579 T:  3956 [             cmd.exe]:                                                      <- ParseRedir (0x0000000000000000)
      1580 T:  3956 [             cmd.exe]:                                                      -> ParseS5
      1581 T:  3956 [             cmd.exe]:                                                          -> ParseCmd
      1582 T:  3956 [             cmd.exe]:                                                              -> LoadNodeTC
      1583 T:  3956 [             cmd.exe]:                                                              <- LoadNodeTC (0x0000013275a2ffc0)
      1584 T:  3956 [             cmd.exe]:                                                              -> Lex
      1585 T:  3956 [             cmd.exe]:                                                                  -> _intrinsic_setjmp
      1586 T:  3956 [             cmd.exe]:                                                                  <- _intrinsic_setjmp (0x0000000000000000)
      1587 T:  3956 [             cmd.exe]:                                                              <- Lex (0x0000000000004000)
      1588 T:  3956 [             cmd.exe]:                                                              -> mkstr
      1589 T:  3956 [             cmd.exe]:                                                              <- mkstr (0x0000013275a2be30)
      1590 T:  3956 [             cmd.exe]:                                                              -> Lex
      1591 T:  3956 [             cmd.exe]:                                                                  -> _intrinsic_setjmp
      1592 T:  3956 [             cmd.exe]:                                                                  <- _intrinsic_setjmp (0x0000000000000000)
      1593 T:  3956 [             cmd.exe]:                                                              <- Lex (0x000000000000000a)
      1594 T:  3956 [             cmd.exe]:                                                              -> ParseRedir
      1595 T:  3956 [             cmd.exe]:                                                              <- ParseRedir (0x0000000000000000)
      1596 T:  3956 [             cmd.exe]:                                                              -> Lex
      1597 T:  3956 [             cmd.exe]:                                                                  -> _intrinsic_setjmp
      1598 T:  3956 [             cmd.exe]:                                                                  <- _intrinsic_setjmp (0x0000000000000000)
      1599 T:  3956 [             cmd.exe]:                                                              <- Lex (0x0000000000000000)
      1600 T:  3956 [             cmd.exe]:                                                          <- ParseCmd (0x0000013275a2ffc0)
      1601 T:  3956 [             cmd.exe]:                                                      <- ParseS5 (0x0000013275a2ffc0)
      1602 T:  3956 [             cmd.exe]:                                                      -> GeToken
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
- finally run the tool `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll -- cmd.exe /c "echo 123; exit"`, it'll create `sym_rt_trace_*.txt` file

# Some hints

To output result to console, add `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsExec --printSymsExecConsole --printSymsModule cmd.exe -- cmd.exe /c "echo 123; exit"`. If your program outputs to console you now have synchronized trace with your logs.

You could track which symbols get instrumented once with `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsInst --printSymsModule cmd.exe -- cmd.exe /c "echo 123; exit"`, this is super fast since does not really affect runtime stage at all.

You could use simple grep feature `drrun.exe -c build\RelWithDebInfo\DrSymLogger.dll --printSymsExec --printSymsExecConsole --printSymsModule cmd.exe --printSymsGrep Parse -- cmd.exe /c "echo 123; exit"`

You cold instrument literally every symbol with anything you want, look at some examples [here]().



