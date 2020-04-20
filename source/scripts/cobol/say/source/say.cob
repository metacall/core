            IDENTIFICATION DIVISION.
            PROGRAM-ID. say.
            ENVIRONMENT DIVISION.
            DATA DIVISION.
            LINKAGE SECTION.
            01 hello PIC X(7).
            01 world PIC X(6).
            PROCEDURE DIVISION USING hello world.
                DISPLAY hello world.
                EXIT PROGRAM.
