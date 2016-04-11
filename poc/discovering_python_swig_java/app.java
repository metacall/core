// java application using extension

public class app {

	static {
		System.loadLibrary("myextension");
	}

	public static void main(String argv[]) {
		System.out.println("Running extension:");

		// run extension
		myextension.run();
	}
}
