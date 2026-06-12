using System.Reflection;
using System.Security.Cryptography.X509Certificates;
using CSLoader.Contracts;
using CSLoader.Providers;

namespace CSLoader.Tests
{
	public class LoaderTests
	{
		public class DummyStatefulTarget
		{
			private int _multiplier = 10;

			public int MultiplyAndAdd(int a, int b)  => (a * b) + _multiplier;

			public void SetMultiplier(int newMultiplier) => _multiplier = newMultiplier;
		}

		public class CapturingLog : ILog
		{
			public List<string> Message { get; } = new List<string>();
			public void Error(string message) => Message.Add($"ERROR: {message}");

			public void Error(string message, Exception ex) =>
				Message.Add($"ERROR: {message} - Exception: {ex.Message}");
			public void Info(string message) => Message.Add($"INFO: {message}");
			public void Debug(string message) => Message.Add($"DEBUG: {message}");
		}

		public static class DummyStaticTarget
		{
			public static int Multiply(int a, int b) => a * b;
		}


		[Fact]
		public void FunctionContainer_ShouldStoreTargetInstanceAndCorrectMethodInfo()
		{
			var targetInstance = new DummyStatefulTarget();
			var methodInfo = typeof(DummyStatefulTarget).GetMethod(nameof(DummyStatefulTarget.MultiplyAndAdd));

			var container = new FunctionContainer(methodInfo, targetInstance);

			Assert.NotNull(container.TargetInstance);
			Assert.Equal("DummyStatefulTarget", container.TargetInstance.GetType().Name);
			// change later when i change the function name to be more dynamic
			Assert.EndsWith("MultiplyAndAdd", container.FunctionName);
			Assert.False(container.Method.IsStatic);
		}

		[Fact]
		public void FunctionContainer_Invoke_ShouldPersistStateChanges()
		{
			var targetInstance = new DummyStatefulTarget();

			var multiplyMethod = typeof(DummyStatefulTarget).GetMethod(nameof(DummyStatefulTarget.MultiplyAndAdd));
			var setMethod = typeof(DummyStatefulTarget).GetMethod(nameof(DummyStatefulTarget.SetMultiplier));

			var multiplyContainer = new FunctionContainer(multiplyMethod, targetInstance);
			var setContainer = new FunctionContainer(setMethod, targetInstance);

			var result1 = multiplyContainer.Method.Invoke(
				multiplyContainer.TargetInstance, new object[] { 5, 2 });

			Assert.Equal(20, (int)result1); // (5 * 2) + 10

			setContainer.Method.Invoke(
				setContainer.TargetInstance, new object[] { 100 });

			var result2 = multiplyContainer.Method.Invoke(
				multiplyContainer.TargetInstance, new object[] { 5, 2 });

			Assert.Equal(110, (int)result2); // (5 * 2) + 100
		}

		[Fact]
		public void FunctionContainer_StaticMethod_ShouldHaveNullTargetInstance()
		{
			var methodInfo = typeof(DummyStaticTarget).GetMethod(nameof(DummyStaticTarget.Multiply));

			var container = new FunctionContainer(methodInfo, null);

			Assert.Null(container.TargetInstance);
			Assert.Equal("Multiply", container.FunctionName);
			Assert.True(container.Method.IsStatic);
		}

		[Fact]
		public void FunctionContainer_StaticMethod_ShouldInvokeCorrectly()
		{
			var methodInfo = typeof(DummyStaticTarget).GetMethod(nameof(DummyStaticTarget.Multiply));
			var container = new FunctionContainer(methodInfo, null);

			var result = container.Method.Invoke(null, new object[] { 5, 2 });

			Assert.Equal(10, (int)result);
		}

		[Fact]
		public void FunctionContainer_TwoInstances_ShouldHaveIndependentState()
		{
			var instance1 = new DummyStatefulTarget();
			var instance2 = new DummyStatefulTarget();

			var setMethod = typeof(DummyStatefulTarget).GetMethod(nameof(DummyStatefulTarget.SetMultiplier));
			var multiplyMethod = typeof(DummyStatefulTarget).GetMethod(nameof(DummyStatefulTarget.MultiplyAndAdd));

			var set1 = new FunctionContainer(setMethod, instance1);
			var set2 = new FunctionContainer(setMethod, instance2);
			var multiply1 = new FunctionContainer(multiplyMethod, instance1);
			var multiply2 = new FunctionContainer(multiplyMethod, instance2);

			// changing only instance1's state
			set1.Method.Invoke(set1.TargetInstance, new object[] { 50 });

			var result1 = multiply1.Method.Invoke(multiply1.TargetInstance, new object[] { 5, 2 });
			var result2 = multiply2.Method.Invoke(multiply2.TargetInstance, new object[] { 5, 2 });

			Assert.Equal(60, (int)result1); // (5 * 2) + 50
			Assert.Equal(20, (int)result2); // (5 * 2) + 10 (default state)

		}

		[Fact]
		public void FunctionContainer_GetReflectFunction_ShouldUseMethodName()
		{
			var methodInfo = typeof(DummyStatefulTarget).GetMethod(nameof(DummyStatefulTarget.MultiplyAndAdd));
			var container = new FunctionContainer(methodInfo, new DummyStatefulTarget());

			var reflectFunction = container.GetReflectFunction();

			Assert.Equal("MultiplyAndAdd", reflectFunction.name);
			Assert.Equal(2, reflectFunction.paramcount);
		}

		[Fact]
		public void LoaderV2EachScriptLoad_ShouldHaveSeperateHandle()
		{
			var log = new CapturingLog();
			var loader = new LoaderV2(log);


			string loaded1 = loader.LoadFromSourceFunctions(new[]
			{
				"public class ScriptA { public static int ValueA () => 42; }"
			});
			string loaded2 = loader.LoadFromSourceFunctions(new []
			{
				"public class ScriptB { public int ValueB () => 84; }"
			});

			Assert.True(loaded1 != null, string.Join("\n", log.Message));
			Assert.True(loaded1 != null, "ScriptA failed to compile");
			Assert.True(loaded2 != null, "ScriptB failed to compile");

			var metadata = loader.GetLoadedFunctionsMetadata();
			Assert.Contains("ValueA", metadata.Keys);
			Assert.Contains("ValueB", metadata.Keys);

			Assert.Equal(2, loader.LoadContextCount);
		}

		[Fact]
		public void LoaderV2_UnloadScript_ShouldRemoveOnlyThatScriptsFunctions()
		{
			var log = new LoaderTests.CapturingLog();
			var loader = new LoaderV2(log);

			string handle1 = loader.LoadFromSourceFunctionsWithHandle(new[]
			{
				"public class ScriptA { public static int ValueA() => 20; }"
			});

			string handle2 = loader.LoadFromSourceFunctionsWithHandle(new[]
			{
				"public class ScriptB { public static int ValueB() => 40; }"
			});

			var before = loader.GetLoadedFunctionsMetadata();
			Assert.True(before.ContainsKey("ValueA"), "ValueA should be loaded before unloading ScriptA" + string.Join("\n", log.Message));
			Assert.True(before.ContainsKey("ValueB"), "ValueB should be loaded before unloading ScriptA" + string.Join("\n", log.Message));

			//Unload only ScriptA
			loader.UnloadScript(handle1);

			var metadata = loader.GetLoadedFunctionsMetadata();
			Assert.DoesNotContain("ValueA", metadata.Keys);
			Assert.Contains("ValueB", metadata.Keys);
		}

		[Fact]
		public void LoaderV2_ALCShouldFullyUnload_AfterUnloadScript()
		{
			var log = new LoaderTests.CapturingLog();
			var loader = new LoaderV2(log);

			string handle = loader.LoadFromSourceFunctionsWithHandle(new[]
			{
				"public class Temp { public static int X() => 20; }"
			});

			loader.UnloadScript(handle);

			Assert.Equal(0, loader.LoadContextCount);
		}
	}
}
