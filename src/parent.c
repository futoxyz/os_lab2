#include <windows.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE 1024

int main(void) {
    char name[256];
    char cmd[300];
    char line[MAX_LINE + 2];

    printf("Enter filename: ");
    if (!fgets(name, sizeof(name), stdin)) {
        fprintf(stderr, "Couldn't read filename\n");
        return 1;
    }
    name[strcspn(name, "\r\n")] = '\0';
    if (name[0] == '\0') {
        fprintf(stderr, "Filename is empty\n");
        return 1;
    }

    if (snprintf(cmd, sizeof(cmd), "child.exe \"%s\"", name) >= sizeof(cmd)) {
        fprintf(stderr, "Filename length is out of range\n");
        return 1;
    }

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE r1, w1;
    HANDLE r2, w2;

    if (!CreatePipe(&r1, &w1, &sa, 0) || !CreatePipe(&r2, &w2, &sa, 0)) {
        fprintf(stderr, "CreatePipe error %lu\n", GetLastError());
        return 1;
    }

    if (!SetHandleInformation(w1, HANDLE_FLAG_INHERIT, 0) || !SetHandleInformation(r2, HANDLE_FLAG_INHERIT, 0)) {
        fprintf(stderr, "SetHandleInformation error %lu\n", GetLastError());
        return 1;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = r1;
    si.hStdOutput = w2;
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess error %lu\n", GetLastError());
        return 1;
    }

    CloseHandle(r1);
    CloseHandle(w2);

    while (fgets(line, MAX_LINE, stdin) != NULL) {
        size_t len = strlen(line);

        if (line[len - 1] != '\n') {
            if (!feof(stdin)) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF) { }
                fprintf(stderr, "Line length is out of range\n");
                continue;
            }
            line[len++] = '\n';
            line[len] = '\0';
        }

        DWORD n;
        if (!WriteFile(w1, line, (DWORD)len, &n, NULL) || n != len) {
            break;
        }
    }

    CloseHandle(w1);
    CloseHandle(r2);

    if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED) {
        fprintf(stderr, "WaitForSingleObject error %lu\n", GetLastError());
        return 1;
    }

    DWORD code;
    if (!GetExitCodeProcess(pi.hProcess, &code)) {
        fprintf(stderr, "GetExitCodeProcess error %lu\n", GetLastError());
        return 1;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (code == 0) {
        return code;
    } else if (code == 2) {
        printf("Division by 0 error\n");
    } else {
        printf("Child finished with code %lu\n", code);
    }
    return code;
}
