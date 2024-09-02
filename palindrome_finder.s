.global _start

// Program Entry Point
_start:
    ldr r0, =input          // Load the address of the input string into r0
    
    bl check_palindrome
    
    // bl check_palindrome     // Call the palindrome checking function
    b _exit                 // Exit when done

// loop through the string backwards, check if the same as forwards
// skip over spaces and ?

// Function: check_palindrome
// Checks if the input string is a palindrome.
check_palindrome:
    // Find the length of the string (excluding the null byte)
    bl find_length          // Call function to find the length, result in r0

    mov r3, #0
    add r3, r1, r2          // r2 now points to the last character of the string
    @ sub r2, r1, #1         // r2 = length of string - 1 (for 0-based index)

    bx lr

find_length:
    mov r1, #0              // Initialize length counter

    bl find_length_loop
    sub r1, r1, #1
    bx lr

find_length_loop:
    ldrb r2, [r0, r1]       // Load a byte from the string at [r1 + r0]
    cmp r2, #0              // Compare the byte to null terminator
    bne .+8                 // If null terminator, branch to done
    bx lr
    add r1, r1, #1          // Increment the length counter
    b find_length_loop      // Repeat until null terminator is found

_exit:
    b .                     // Infinite loop to halt the program

// Data Section
.data
.align
input:              .asciz "Grav ned den varg"     // The input string to check

msg_palindrome:     .asciz "Palindrome detected" // Message for palindrome
msg_not_palindrome: .asciz "Not a palindrome"    // Message for not palindrome

.end
