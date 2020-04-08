using System;
namespace Scripts
{
	public class JumpMaster
	{
		public static int SuperJump()
		{
			IJump sj = new SuperJump();
			return sj.Jump();
		}

		public static int TinyJump()
		{
			IJump sj = new TinyJump();
			return sj.Jump();
		}
	}
}
