int fact(int n) {
	if (n == 1)
		return n;
	else
		return (n*fact(n-1));
}
int main() {
	int m, res;
	m = read();
	if (m > 1)
		res = fact(m);
	else
		res = 1;
	write(res);
	return 0;
}
