using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StartBZFS
{
    public partial class ServerLogWindow : Form
    {
        public bool stop = false;

        public ServerLogWindow()
        {
            InitializeComponent();
            stop = false;
        }
        
        public void addText ( string line )
        {
            ServerLog.Text += line + "\r\n";
            ServerLog.Select(ServerLog.Text.Length-1, 1);
            ServerLog.ScrollToCaret();
        }

        private void ServerLogWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            stop = true;
        }

        private void Stop_Click(object sender, EventArgs e)
        {
            stop = true;
        }
    }
}
