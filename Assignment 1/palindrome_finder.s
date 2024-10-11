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

    // load the address of the input string into r4
    ldr r4, =input
    @ ldr r4, =long_enough
    @ ldr r4, =not_long_enough // this will cause the program to halt immediately
    @ ldr r4, =palindrome_1
    @ ldr r4, =palindrome_2
    @ ldr r4, =palindrome_3
    @ ldr r4, =palindrome_4
    @ ldr r4, =palindrome_5
    @ ldr r4, =palindrome_6
    @ ldr r4, =palindrome_7
    @ ldr r4, =palindrome_8
    @ ldr r4, =palindrome_9
    @ ldr r4, =not_palindrome_1
    @ ldr r4, =not_palindrome_2
    
    bl find_length_loop
    bl check_long_enough

    // save the length of the string in r5 (length of string - 1)
    mov r8, r1
    sub r5, r1, #1

    // set r2 to the beginning of the string and r3 to the end
    mov r2, #0
    mov r3, r5

    bl check_palindrome_loop

    bl is_not_palindrome
    bl is_palindrome

    // save the address for jtag
    ldr r1, =0xFF201000
    mov r3, #0
    bl write_jtag_loop

    // halt the program
    b _exit

write_jtag_loop:
    // load the byte at the address in r0 + r3 into r2
    // r0 is the address of palindrome_detected or not_a_palindrome
    ldrb r2, [r0, r3]
    cmp r2, #0
    bne .+8
    // return if null terminator
    bx lr
    
    // write the byte in r2 to the jtag
    str r2, [r1]
    // increment r3 and repeat
    add r3, r3, #1
    b write_jtag_loop

find_length_loop:
    // load the byte at the address in r4 + r1 into r2
    ldrb r2, [r4, r1]
    // compare the byte to null terminator
    cmp r2, #0
    bne .+8
    // return if null terminator
    bx lr
    
    // increment the length counter and repeat
    add r1, r1, #1
    b find_length_loop

check_long_enough:
    // check if length is at least 2
    cmp r1, #1
    bgt .+8
    b _exit
    bx lr

check_palindrome_loop:
    // load the bytes from the beginning and end of the string
    ldrb r0, [r4, r2]
    ldrb r1, [r4, r3]

    // check if we have reached the end
    cmp r2, r8
    blt .+8
    bx lr

    // -----------
    // check for ?
    // -----------
    // check if one of the characters is a ?
    cmp r0, #63
    bne .+16
    // if it is, increment the beginning and decrement the end
    add r2, r2, #1
    sub r3, r3, #1
    b check_palindrome_loop

    // repeat for the other character
    cmp r1, #63
    bne .+16
    add r2, r2, #1
    sub r3, r3, #1
    b check_palindrome_loop

    // ---------------
    // check for space
    // ---------------
    // check if one of the characters is a space
    cmp r0, #32
    bne .+12
    // if it is, increment the beginning
    add r2, r2, #1
    b check_palindrome_loop

    // repeat for the other character
    cmp r1, #32
    bne .+12
    sub r3, r3, #1
    b check_palindrome_loop

    // ------------------
    // check for uppercase
    // -------------------
    // check if A <= r0 <= Z
    // if it is, convert to lowercase
    // 90 is Z
    cmp r0, #90             
    bgt .+16
    // 65 is A (skip if less than A)
    cmp r0, #64
    blt .+8
    add r0, #32

    // --------------------------
    // compare the two characters
    // --------------------------
    // repeat for the other character
    cmp r1, #90
    bgt .+16
    cmp r1, #64
    blt .+8
    add r1, #32 

    // here both characters are lowercase or numbers
    cmp r0, r1
    bne .+20
    // r12 is result, if 1 then palindrome
    mov r12, #1
    // continue the loop
    add r2, r2, #1
    sub r3, r3, #1
    b check_palindrome_loop

    // r12 is result, if 0 then not a palindrome
    // we can then break, no need to check the rest
    mov r12, #0
    bx lr

is_not_palindrome:
    // actually check if it is not a palindrome
    cmp r12, #0
    beq .+8 
    bx lr

    // set the leftmost leds
    ldr r2, =0xFF200000
    ldr r3, =0b1111100000
    str r3, [r2]

    // save the correct string address for jtag
    ldr r0, =not_a_palindrome
    bx lr

is_palindrome:
    cmp r12, #1
    beq .+8 
    bx lr

    // set the rightmost leds
    ldr r2, =0xFF200000
    ldr r3, =0b0000011111
    str r3, [r2]

    // save the corect string address for jtag
    ldr r0, =palindrome_detected
    bx lr

_exit:
    // infinite loop to halt the program
    b .

// Data Section
.data
.align
    input:                  .asciz "Grav ned den varg"
    // strings for jtag
    palindrome_detected:    .asciz "Palindrome detected"
    not_a_palindrome:       .asciz "Not a palindrome"
    // length tests
    long_enough:            .asciz "mm"
    not_long_enough:        .asciz "m" 
    // valid palindromes
    palindrome_1:           .asciz "level"
    palindrome_2:           .asciz "8448"
    palindrome_3:           .asciz "step on no pets"
    palindrome_4:           .asciz "My gym"
    palindrome_5:           .asciz "va?lliav"
    palindrome_6:           .asciz "A santa at NASA"
    palindrome_7:           .asciz "1991?sos 1991"
    palindrome_8:           .asciz "48aBa84"
    palindrome_9:           .asciz "19AZza91"
    // invalid palindromes
    not_palindrome_1:       .asciz "hello world"
    not_palindrome_2:       .asciz "A santa"

.end
