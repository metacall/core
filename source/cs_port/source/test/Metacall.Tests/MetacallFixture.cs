namespace Metacall.Tests
{
    public class MetacallFixture
    {
        protected MetacallCsPort.Metacall Metacall { get; private set; }

        protected dynamic Caller { get; private set; }

        public MetacallFixture()
        {
            this.Metacall = new MetacallCsPort.Metacall();
            this.Caller = this.Metacall;
        }
    }
}
