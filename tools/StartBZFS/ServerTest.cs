using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Web;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Diagnostics;

namespace StartBZFS
{
    public partial class ServerTest : Form
    {
        public string address = string.Empty;
        public int port = 5154;

        string testURL = "http://my.bzflag.org/servertest.php?";
        public ServerTest()
        {
            InitializeComponent();
            
        }

        void LogLine(string text)
        {
            Log.Text += text + "\r\n";
            Log.Select(Log.Text.Length - 1, 1);
            Application.DoEvents();
        }

        private void ServerTest_Shown(object sender, EventArgs e)
        {
            LogLine("Starting Test");

            string URL = testURL;
            if (address != string.Empty)
                URL += "address=" + address + "&";
            if (port > 0)
                URL += "port=" + port.ToString() + "&";
            else
                port = 5154;

            Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.IP);

            LogLine("Contacting");
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(URL);
            HttpWebResponse response = (HttpWebResponse)request.GetResponse();
            LogLine("Contacted");

            StreamReader sr = new StreamReader(response.GetResponseStream());
            LogLine("Response");

            string line;
            while ((line = sr.ReadLine()) != null)
            {
                LogLine(line);
                Application.DoEvents();
            }
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start("http://www.portforward.com");
        }
    }
}
