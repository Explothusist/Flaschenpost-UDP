REM REM - comment

cd Flpt
make
cd ..
REM /Y - auto override
copy Flpt\Flaschenpost.dll /Y
copy Flpt\libFlaschenpost.a /Y

REM && - if returns 0, || - if returns 1
make client.exe || (
    REM echo - print to console
    echo Build Failed.
    REM pause - wait until user input
    pause 
    REM /b - only exits the script, leaves cmd open, 1 - error code
    exit /b 1 
)

REM runs the new exe file
client 