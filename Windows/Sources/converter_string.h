#ifndef converter_string_H
#define converter_string_H

#include <ntifs.h>
#include <ntddk.h>


/*UNICODE_STRING --> ANSI_STRING*/
NTSTATUS UNICODE_to_ANSI(ANSI_STRING* OUTPUT_ansi, UNICODE_STRING* INPUT_unicode);
VOID UNICODE_to_ANSI_release(ANSI_STRING* INPUT_ansi);

/*ANSI_STRING --> UNICODE_STRING*/
NTSTATUS ANSI_to_UNICODE(UNICODE_STRING* OUTPUT_unicode, ANSI_STRING INPUT_ansi);
VOID ANSI_to_UNICODE_release(UNICODE_STRING* INPUT_unicode);

/* PWCH -> ANSI_STRING */
NTSTATUS PWCH_to_ANSI(ANSI_STRING* OUTPUT_ansi, PWCH INPUT_pwch);
VOID PWCH_to_ANSI_release(ANSI_STRING* INPUT_ansi);

#endif