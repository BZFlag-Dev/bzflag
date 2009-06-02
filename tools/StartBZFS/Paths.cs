using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace StartBZFS
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

            ServerPath.Text = prefs.ServerPath;
            WorldPath.Text = prefs.WorldPath;
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
            ofd.AutoUpgradeEnabled = true;
            ofd.Multiselect = false;
            if (ServerPath.Text != string.Empty)
                ofd.InitialDirectory = Path.GetDirectoryName(ServerPath.Text);
            ofd.FileName = "bzfs";
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
            if (WorldPath.Text != string.Empty)
                ofd.InitialDirectory = Path.GetDirectoryName(WorldPath.Text);
            ofd.FileName = "*.bzw";
            if (ofd.ShowDialog() == DialogResult.OK)
                WorldPath.Text = Path.GetDirectoryName(ofd.FileName);
        }

        private void OK_Click(object sender, EventArgs e)
        {
            prefs.ServerPath = ServerPath.Text;
            prefs.WorldPath = WorldPath.Text;
        }
    }
}
