using MetacallCsPort;
using System;
using Xunit;
using FluentAssertions;

namespace Metacall.Tests
{
    public class MetacallTest : MetacallFixture
    {
        static MetacallTest()
        {
            var metacall = new MetacallCsPort.Metacall();
            metacall.LoadScriptFromFile("py", "example.py").Should().BeTrue();
            metacall.LoadScriptFromFile("cs", "hello.cs").Should().BeTrue();
        }

        [Fact]
        public void CallSayHelloFunction()
        {
            using (ExecutionResult er = this.Caller.SayHello())
            {
            }
        }

        [Fact]
        public void CallSumFunction()
        {
            using (ExecutionResult er = this.Caller.Sum(1, 2))
            {
                er.HasValue.Should().BeTrue();
                er.AsInt().Should().Be(3);
            }
        }

        [Fact]
        public void GetFunctionNotFonund()
        {
            this.Metacall.HasFunction("NotExist").Should().BeFalse();
        }

        [Fact]
        public void CallFunctionFromGetFunction()
        {
            IFunction function = this.Metacall.GetFuntion("Sum");

            function.Should().NotBeNull();

            using(var result = function.Invoke(1, 2))
            {
                result.Should().NotBeNull();
                result.HasValue.Should().BeTrue();
                result.AsInt().Should().Be(3);
            }
        }
    }
}
