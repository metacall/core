package metacall;

import java.nio.file.Paths;
import java.util.*;
import com.sun.jna.ptr.PointerByReference;

import metacall.util.*;

class Loader implements Runtime{
	int code;
	void loadFilesUnsafe(Runtime runtime, Vector<String> filePaths, Optional<PointerByReference> handleRef) throws Exception
	{
		//TODO
		// absolutePaths
		// code

		if (code != 0)
		{
			throw new Exception("Failed to load scripts" + Arrays.toString(filePaths.toArray()));
		}
		else
		{
			System.out.println("Success");
		}
	}
}

interface Runtime
{
	class Python
	{
		private Python() {}
		@Override
		public String toString() {
			return "py";
		}
	}

	class Node
	{
		private Node() {}
		@Override
		public String toString() {
			return "node";
		}
	}
}
