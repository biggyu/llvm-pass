float foo(float a, float b) {
	return a * b + a / b;
}

int main(int argc, char *argv[]) {
    int a = 123;
    int b = 4;

    return foo(a, b);
}