def check_input(string: str) -> bool:
    if len(string) < 2:
        return False

    return True


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


def main() -> None:
    string = "Grav ned den varg"
    # string = "K aYak"

    is_valid_string = check_input(string)

    if not is_valid_string:
        print("Input has to be longer than 2 characters")
        return

    print(is_palindrome(string))


if __name__ == "__main__":
    main()
