using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace BZFStart
{
    public partial class Paths : Form
    {
        DirectoryInfo configDir;
        Prefrences prefs;

        public Paths( DirectoryInfo dir, Prefrences _prefs )
        {
            configDir = dir;
            prefs = _prefs;
            InitializeComponent();

            ClientPath.Text = prefs.ClientPath;
            ServerPath.Text = prefs.ServerPath;
            WorldPath.Text = prefs.WorldPath;
        }

        private void AutoFindClient_Click(object sender, EventArgs e)
        {
            FileInfo client = Prefrences.FindClient(configDir);
            if (!client.Exists)
            {
                AutoFindClient.Enabled = false;
                MessageBox.Show("The client could not be automatically found");
            }
            else
                ClientPath.Text = client.FullName;
        }

        private void BrowseClient_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.CheckFileExists = true;
            ofd.Multiselect = false;
            ofd.FileName = Prefrences.FindClient(configDir).FullName;
            if (ofd.ShowDialog() == DialogResult.OK)
                ClientPath.Text = ofd.FileName;
        }

        private void AutoFindServer_Click(object sender, EventArgs e)
        {
            FileInfo server = Prefrences.FindServer(configDir);
            if (!server.Exists)
            {
                AutoFindServer.Enabled = false;
                MessageBox.Show("The server could not be automatically found");
            }
            else
                ServerPath.Text = server.FullName;
        }

        private void BrowseServer_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.CheckFileExists = true;
            ofd.Multiselect = false;
            ofd.FileName = Prefrences.FindServer(configDir).FullName;
            if (ofd.ShowDialog() == DialogResult.OK)
                ServerPath.Text = ofd.FileName;
        }

        private void AutoFindWorld_Click(object sender, EventArgs e)
        {
            DirectoryInfo worldDir = Prefrences.FindWorldDir(configDir);
            if (!worldDir.Exists)
            {
                AutoFindWorld.Enabled = false;
                MessageBox.Show("The world directory could not be automatically found");
            }
            else
                WorldPath.Text = worldDir.FullName;

        }

        private void BrowseWorld_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Multiselect = false;
            ofd.CheckFileExists = true;
            ofd.Filter = "BZFlag Map files (*.bzw)|*.bzw|All files (*.*)|*.*";
            ofd.FileName = Path.Combine(Prefrences.FindServer(configDir).FullName,"*.bzw");
            if (ofd.ShowDialog() == DialogResult.OK)
                ServerPath.Text = Path.GetDirectoryName(ofd.FileName);
        }

        private void OK_Click(object sender, EventArgs e)
        {
            prefs.ClientPath = ClientPath.Text;
            prefs.ServerPath = ServerPath.Text;
            prefs.WorldPath = WorldPath.Text;
        }
    }
}
