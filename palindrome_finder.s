.global _start

_start:
    // reset registers for easier debugging
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
    @ ldr r4, =long_enough
    @ ldr r4, =not_long_enough
    @ ldr r4, =palindrome_1
    @ ldr r4, =palindrome_2
    @ ldr r4, =palindrome_3
    @ ldr r4, =palindrome_4
    @ ldr r4, =palindrome_5
    @ ldr r4, =palindrome_6
    @ ldr r4, =palindrome_7
    @ ldr r4, =palindrome_8
    @ ldr r4, =not_palindrome_1
    @ ldr r4, =not_palindrome_2
    
    bl find_length_loop
    bl check_long_enough

    mov r8, r1              // keep orginal counter
    sub r5, r1, #1          // length of string - 1

    mov r2, #0
    mov r3, r5

    bl check_palindrome_loop

    bl is_not_palindrome
    bl is_palindrome

    ldr r1, =0xFF201000
    mov r3, #0
    bl write_jtag_loop

    // end
    b _exit

write_jtag_loop:
    ldrb r2, [r0, r3]
    cmp r2, #0
    bne .+8
    bx lr
    
    str r2, [r1]
    add r3, r3, #1
    b write_jtag_loop

find_length_loop:
    ldrb r2, [r4, r1]       // load a byte from the string at [r1 + r0]
    cmp r2, #0              // compare the byte to null terminator
    bne .+8                 // if null terminator, branch to done
    bx lr
    add r1, r1, #1          // increment the length counter
    b find_length_loop      // repeat until null terminator is found

check_long_enough:
    cmp r1, #2
    bgt .+8
    b _exit
    bx lr

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
    bgt .+16
    cmp r0, #65             // 65 is A
    blt .+8
    add r0, #32             // A < r0 < Z

    cmp r1, #90
    bgt .+16
    cmp r1, #65
    blt .+8
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

    // set leds
    ldr r2, =0xFF200000
    ldr r3, =0b1111100000
    str r3, [r2]

    // save for jtag
    ldr r0, =not_a_palindrome
    
    bx lr

is_palindrome:
    cmp r12, #1
    beq .+8 
    bx lr

    // set leds
    ldr r2, =0xFF200000
    ldr r3, =0b0000011111
    str r3, [r2]

    // save for jtag
    ldr r0, =palindrome_detected
    
    bx lr

_exit:
    b .                     // Infinite loop to halt the program

// Data Section
.data
.align
    input:                  .asciz "Grav ned den varg"     // The input string to check
    palindrome_detected:    .asciz "Palindrome detected"
    not_a_palindrome:       .asciz "Not a palindrome"
    long_enough:            .asciz "mim"
    not_long_enough:        .asciz "m" 
    palindrome_1:           .asciz "level"
    palindrome_2:           .asciz "8448"
    palindrome_3:           .asciz "step on no pets"
    palindrome_4:           .asciz "My gym"
    palindrome_5:           .asciz "va?lliav"
    palindrome_6:           .asciz "A santa at NASA"
    palindrome_7:           .asciz "1991?sos 1991"
    palindrome_8:           .asciz "48aBa84"
    not_palindrome_1:       .asciz "hello world"
    not_palindrome_2:       .asciz "A santa"

.end
