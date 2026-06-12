REM REM -> comment
make client.exe || ( REM && -> if returns 0, || -> if returns 1
    REM echo -> print to console
    echo Build Failed.
    pause REM pause -> wait until user input
    exit /b 1 REM /b -> only exits the script, leaves cmd open, 1 -> error code
)

client REM runs the new exe file