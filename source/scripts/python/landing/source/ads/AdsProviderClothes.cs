using System;

namespace Scripts
{
	public class AdsProviderClothes : IAdsProvider
	{
		private static Random _random = new Random();
		private static AdsImpression[] _impressions = {
			new AdsImpression("Shop women's boots, men's boots, kids' shoes, industrial footwear, leather bags and accessories at Dr. Martens official site.", "https://www.drmartens.com"),
			new AdsImpression("Latest trends in clothing for women, men & kids at ZARA online.", "https://www.zara.com"),
			new AdsImpression("Buy and sell 100% authentic Supreme streetwear apparel and accessories at the best price on StockX.", "https://stockx.com/streetwear"),
			new AdsImpression("Discover the latest fashion and trends in menswear and womenswear at ASOS. Shop this season's collection of clothes, accessories, beauty and more.", "https://www.asos.com/"),
			new AdsImpression("Get the latest womens fashion online at boohoo.com. With 100s of new styles every day from dresses, onesies, heels, & coats, shop womens clothing now!", "https://nl.boohoo.com/"),
			new AdsImpression("Clothes, The flea market is the ideal place to find basic pieces and discover great treasures.", "https://lasdalias.es/en/c/ropa"),
			new AdsImpression("Clothing - Shop Online from trendy apparels for women, men & kids at best prices. Select your favourite clothing from the fashionable collection on Myntra.", "https://www.myntra.com/clothing"),
		};

		public AdsImpression Generate()
		{
			return _impressions[_random.Next(_impressions.Length)];
		}
	}
}
