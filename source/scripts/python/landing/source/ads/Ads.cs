using System;

namespace Scripts
{
	public class Program
	{
		public static string Ads()
		{
			AdsProviderClothes provider = new AdsProviderClothes();
			AdsImpression impression = provider.Generate();

			return $"<div><p>{impression.Message}</p><a>{impression.URL}</a></div>";
		}

		public static void Main(string[] args)
		{
			AdsProviderClothes c = new AdsProviderClothes();

			Console.WriteLine(Ads());
		}
	}
}
