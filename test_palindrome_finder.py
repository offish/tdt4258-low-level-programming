from palindrome_finder import is_palindrome


def test_palindromes():
    valid_palindromes = [
        "level",
        "8448",
        "step on no pets",
        "My gym",
        "Was it a car or a cat i saw",
        "va?lliav",
        "A santa at NASA",
    ]

    for palindrome in valid_palindromes:
        assert is_palindrome(palindrome)


def test_invalid_palindromes():
    invalid_palindromes = ["First level", "Palindrome", "A santa"]

    for palindrome in invalid_palindromes:
        assert not is_palindrome(palindrome)
