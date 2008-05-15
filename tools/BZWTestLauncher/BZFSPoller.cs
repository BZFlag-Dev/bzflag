using System;
using System.Windows.Forms;
using System.Threading;

namespace BZWTestLauncher
{
	/// <summary>
	/// Summary description for BZFSPoller.
	/// </summary>
	public class BZFSPoller
	{
		public BZFSPoller(System.IO.StreamReader _sr, bool _err)
		{
			sr = _sr;
			err = _err;
		}

		private System.IO.StreamReader sr;
		private bool err;

		private static RichTextBox outText;
		public static RichTextBox OutputBox
		{
			set
			{
				outText = value;
			}
		}

		private static bool exit;
		public static bool Exit // write-only
		{
			set
			{
				exit = value;
			}
		}

		private static string outQueue = "";

		// each poller has its own thread
		public void pollBZFS()
		{
			string output = "";
			// ReadLine blocks the thread
			while ((output = sr.ReadLine()) != null)
			{
				if (!err)
					output = Rtfify(output) + "\\par ";
				else
					output = "{\\b " + Rtfify(output) + "}\\par ";

				queue(output);
			}
		}

		// queue may only be accessed by one poller or the collator at a time
		public static void queue(string output)
		{
			lock (outQueue)
			{
				outQueue += output;
			}
		}

		// collator runs in its own thread
		public static void collator()
		{
			exit = false;
			while (!exit)
			{
				lock (outQueue)
				{
					if (outQueue.Length > 0)
					{
						outText.Invoke(new appendToRTF(genericAppendToRTF), 
							new object[] { outText, outQueue });
						outQueue = "";
					}
				}
				// Refresh the RTF box at no more than 20 Hz
				Thread.Sleep(50);
			}
		}

		static private string Rtfify(string instr)
		{
			return instr
				.Replace(@"\", @"\\")
				.Replace("}", "\\}")
				.Replace("{", "\\{")
				.Replace("\r\n", "\\par ")
				.Replace("\r", "\\par ")
				.Replace("\n", "\\par ");
		}

		// appendToRTF runs in the main thread (via Invoke)
		private delegate void appendToRTF(object sender, string what);
		private static void genericAppendToRTF(object sender, string what)
		{
			RichTextBox rte = sender as RichTextBox;
			if (rte == null) return;
			rte.Rtf = rte.Rtf.Insert(rte.Rtf.LastIndexOf('}') - 1, what);
			rte.Focus();
			rte.Select(rte.Text.Length, 0);
			rte.ScrollToCaret();
		}
	}
}
