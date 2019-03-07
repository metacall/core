namespace Scripts
{
	public class AdsImpression
	{
		private string _message;
		private string _url;

		public AdsImpression(string message, string url)
		{
			_message = message;
			_url = url;
		}

		public string Message
		{
			get
			{
				return _message;
			}
		}

		public string URL
		{
			get
			{
				return _url;
			}
		}
	}
}
