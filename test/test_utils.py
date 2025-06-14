def print_separator(title=None):
    sep = "\n------\n"
    if title:
        print(f"{sep}{title}{sep}")
    else:
        print(sep)
