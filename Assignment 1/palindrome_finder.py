def is_palindrome(string: str) -> bool:
    # case insensitive
    string = string.lower()
    # remove whitespace
    string = string.replace(" ", "")

    for i, char in enumerate(string):
        compare_char = string[len(string) - i - 1]

        # ? is a wildcard
        if char == "?" or compare_char == "?":
            continue

        if not char.isalnum():
            return False

        if char != compare_char:
            return False

    return True
