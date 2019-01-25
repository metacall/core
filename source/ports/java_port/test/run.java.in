public class main {
	static {
		try {
			System.loadLibrary("metacall");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Native code library failed to load. " + e);
			System.exit(1);
		}
	}

	public static void main(String argv[]) {
		System.out.println(metacall.metacall("multiply", 4, 4));
	}
}
