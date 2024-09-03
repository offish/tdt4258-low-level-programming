.global _start

// Program Entry Point
_start:
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0

    ldr r4, =input
    @ ldr r4, =palindrome_1
    @ ldr r4, =palindrome_2
    @ ldr r4, =palindrome_3
    @ ldr r4, =palindrome_4
    @ ldr r4, =palindrome_5
    @ ldr r4, =palindrome_6
    @ ldr r4, =palindrome_7
    @ ldr r4, =not_palindrome_1
    @ ldr r4, =not_palindrome_2
    
    bl find_length_loop

    mov r8, r1              // keep orginal counter
    sub r5, r1, #1          // length of string - 1

    mov r2, #0
    mov r3, r5

    bl check_palindrome_loop

    bl is_not_palindrome
    bl is_palindrome

    // end
    b _exit

find_length_loop:
    ldrb r2, [r4, r1]       // load a byte from the string at [r1 + r0]
    cmp r2, #0              // compare the byte to null terminator
    bne .+8                 // if null terminator, branch to done
    bx lr
    add r1, r1, #1          // increment the length counter
    b find_length_loop      // repeat until null terminator is found

check_palindrome_loop:
    ldrb r0, [r4, r2]
    ldrb r1, [r4, r3]

    // check if done
    cmp r2, r8
    blt .+8
    bx lr

    cmp r0, #63            // check for ?
    bne .+16
    add r2, r2, #1
    sub r3, r3, #1
    b check_palindrome_loop

    cmp r1, #63
    bne .+16
    add r2, r2, #1
    sub r3, r3, #1
    b check_palindrome_loop

    cmp r0, #32            // 32 is space
    bne .+12
    add r2, r2, #1
    b check_palindrome_loop

    cmp r1, #32
    bne .+12
    sub r3, r3, #1
    b check_palindrome_loop

    // check if uppercase, make lowercase
    cmp r0, #90             // 90 is Z
    bgt .+8
    add r0, #32             // 122 is z

    cmp r1, #90
    bgt .+8
    add r1, #32 

    // now everything is lowercase
    cmp r0, r1
    bne .+20
    mov r12, #1              // is palindrome
    add r2, r2, #1
    sub r3, r3, #1
    b check_palindrome_loop

    mov r12, #0
    bx lr


is_not_palindrome:
    cmp r12, #0
    beq .+8 
    bx lr

    // is not palindrome
    mov r0, #0
    bx lr

is_palindrome:
    cmp r12, #1
    beq .+8 
    bx lr

    // is palindrome
    mov r0, #1
    bx lr

_exit:
    b .                     // Infinite loop to halt the program

// Data Section
.data
.align
    input:              .asciz "Grav ned den varg"     // The input string to check

    palindrome_1:       .asciz "level"
    palindrome_2:       .asciz "8448"
    palindrome_3:       .asciz "step on no pets"
    palindrome_4:       .asciz "My gym"
    palindrome_5:       .asciz "Was it a car or a cat i saw"
    palindrome_6:       .asciz "va?lliav"
    palindrome_7:       .asciz "A santa at NASA"
    not_palindrome_1:   .asciz "hello world"
    not_palindrome_2:   .asciz "A santa"

.end
