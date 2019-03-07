namespace Scripts
{
	public class AdsProviderGambling : IAdsProvider
	{
		public AdsImpression Generate()
		{
			return new AdsImpression("AdsProviderGambling", "https://gambling.com");
		}
	}
}
