using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;
using System.IO;

namespace StartBZFS
{
    public partial class Form1 : Form
    {
        Prefrences prefs;
        DirectoryInfo confDir;

        ServerConfig serverConfig = new ServerConfig();

        List<FileInfo> maps = new List<FileInfo>();

        bool settingMaps = false;

        int findMapIndex(string name)
        {
            int index = 0;
            foreach (FileInfo f in maps)
            {
                if (f.FullName == name)
                    return index;
                index++;
            }
            return -1;
        }

        public Form1()
        {
            InitializeComponent();
            loadPrefs();
            scanMaps();

            loadFormFromConfig(serverConfig);
        }

        private void scanMaps()
        {
            maps.Clear();
			if (prefs.WorldPath == string.Empty || prefs.WorldPath.Length == 0)
				return;

            DirectoryInfo mapDir = new DirectoryInfo(prefs.WorldPath);
            if (!mapDir.Exists)
                return;

            foreach (FileInfo f in mapDir.GetFiles("*.bzw"))
                maps.Add(f);
        }

        private void loadFormFromConfig (ServerConfig config)
        {
            Password.Text = string.Empty;

            FFAMode.Checked = config.mode == GameMode.FFA;
            OFFAMode.Checked = config.mode == GameMode.OpenFFA;
            CTFMode.Checked = config.mode == GameMode.CTF;
            RabbitModeItem.Checked = config.mode == GameMode.Rabbit;
            switch(config.rabbitMode)
            {
                case RabbitMode.ScoreBased:
                    RabbitModeType.SelectedIndex = 0;
                    break;
                case RabbitMode.KillerBased:
                    RabbitModeType.SelectedIndex = 1;
                    break;
                case RabbitMode.Random:
                    RabbitModeType.SelectedIndex = 2;
                    break;
            }
            RabbitModeType.Enabled = config.mode == GameMode.Rabbit;

            ServerAddress.Text = config.serverAddress;
            ServerPort.Text = config.port.ToString();

            NumShots.SelectedIndex = config.shots - 1;
            LogLevel.SelectedIndex = config.debugLevel;

            Ricochet.Checked = config.rico;
            Jumping.Checked = config.jumping;

            RandomHeight.Checked = config.randHeight;
            RandomRot.Checked = config.randRot;

            GoodFlags.Checked = config.goodFlags;
            BadFlags.Checked = config.badFlags;
            Antidote.Checked = config.andidote;
            FlagsOnBuildings.Checked = config.flagsOnBuildings;

            Handicap.Checked = config.handicap;

            if (config.maxPlayers > 0)
                MaxPlayers.Text = config.maxPlayers.ToString();

            if (config.maxSuperflags <= 0)
                MaxSuperFlags.SelectedIndex = 0;
            else
            {
                MaxSuperFlags.SelectedIndex = 1; // 5
                if (config.maxSuperflags <= 10)    
                    MaxSuperFlags.SelectedIndex = 2; // 10
                if (config.maxSuperflags <= 15)
                    MaxSuperFlags.SelectedIndex = 3; // 15
                if (config.maxSuperflags <= 20)
                    MaxSuperFlags.SelectedIndex = 4; // 20
                if (config.maxSuperflags <= 25)
                    MaxSuperFlags.SelectedIndex = 5; // 25
                if (config.maxSuperflags <= 30)
                    MaxSuperFlags.SelectedIndex = 6; // 30
                if (config.maxSuperflags <= 35)
                    MaxSuperFlags.SelectedIndex = 7; // 35
                if (config.maxSuperflags <= 40)
                    MaxSuperFlags.SelectedIndex = 8; // 40
            }

            if (config.shakeWins > 0)
                ShakeWins.Text = config.shakeWins.ToString();
            if (config.shakeTime > 0)
                ShakeTime.Text = config.shakeTime.ToString();

            int index = 0;
            if (config.worldfile != string.Empty)
            {
                FileInfo f = new FileInfo(config.worldfile);
                if (!f.Exists)
                    config.worldfile = string.Empty;
                else
                {
                    index = findMapIndex(config.worldfile);
                    if (index < 0)
                    {
                        index = maps.Count;
                        maps.Add(f);
                    }
                }
            }
            buildWorldsList(index);

            Teleporters.Enabled = config.worldfile == string.Empty;
            Teleporters.Checked = config.teleporters;
            SpawnOnBoxes.Checked = config.spawnOnBoxes;

            ResetOnQuit.Checked = !config.quitOngame;

            AutoTeam.Checked = config.autoTeam;

            if (config.endType == GameEndType.Never)
                GameEnds.SelectedIndex = 0;
            else if (config.endType == GameEndType.TimeLimit && config.endValue > 0)
            {
                GameEnds.SelectedIndex = 1; // 5 min
                if (config.endValue >= 15)
                     GameEnds.SelectedIndex = 2; // 15 min
                if (config.endValue >= 60)
                     GameEnds.SelectedIndex = 3; // 60 min
                if (config.endValue >= 180)
                     GameEnds.SelectedIndex = 4; // 3 hours
            }
            else if (config.endType == GameEndType.PlayerScoreLimit && config.endValue > 0)
            {
                GameEnds.SelectedIndex = 5; // 3 pts
                if (config.endValue >= 10)
                    GameEnds.SelectedIndex = 6; // 10 pts
                if (config.endValue >= 25)
                    GameEnds.SelectedIndex = 7; // 25 pts
            }
            else if (config.endType == GameEndType.TeamScoreLimit && config.endValue > 0)
            {
                GameEnds.SelectedIndex = 8; // 3 pts
                if (config.endValue >= 10)
                    GameEnds.SelectedIndex = 9; // 10 pts
                if (config.endValue >= 25)
                    GameEnds.SelectedIndex = 10; // 25 pts
                if (config.endValue >= 100)
                    GameEnds.SelectedIndex = 11; // 100 pts
            }
            PublicServer_CheckedChanged(this, EventArgs.Empty);
            checkAddressItem();
            checkServerStartButton();
        }

        private void setConfigFromForm (ServerConfig config)
        {
            config.password = Password.Text;

            if(FFAMode.Checked)
                config.mode = GameMode.FFA;
            else if(OFFAMode.Checked)
                config.mode = GameMode.OpenFFA;
            else if (CTFMode.Checked)
                config.mode = GameMode.CTF;
            else if (RabbitModeItem.Checked)
            {
                config.mode = GameMode.Rabbit;
                config.rabbitMode = (RabbitMode)RabbitModeType.SelectedIndex;
            }

            config.shots = NumShots.SelectedIndex + 1;
            config.debugLevel = LogLevel.SelectedIndex;

            config.rico = Ricochet.Checked;
            config.jumping = Jumping.Checked;

            config.randHeight = RandomHeight.Checked;
            config.randRot = RandomRot.Checked;

            config.andidote = Antidote.Checked;
            config.flagsOnBuildings = FlagsOnBuildings.Checked;

            config.handicap = Handicap.Checked;

            if (MaxPlayers.Text != string.Empty)
                config.maxPlayers = int.Parse(MaxPlayers.Text);

            config.goodFlags = GoodFlags.Checked;
            config.badFlags = BadFlags.Checked;
            if (ShakeWins.Text != string.Empty)
            {
                int i = int.Parse(ShakeWins.Text);
                if ( i > 0)
                    config.shakeWins = i;
                else
                    config.shakeWins = 0;
            }

            if (ShakeTime.Text != string.Empty)
            {
                int i = int.Parse(ShakeWins.Text);
                if ( i > 0)
                    config.shakeTime = i;
                else
                    config.shakeTime = 0;
            }

            config.teleporters = Teleporters.Checked;
            config.spawnOnBoxes = SpawnOnBoxes.Checked;

            if (WorldsList.SelectedIndex == 0)
                config.worldfile = string.Empty;
            else
                config.worldfile = maps[WorldsList.SelectedIndex - 1].FullName;

            config.publicServer = PublicServer.Checked;
            if (config.publicServer)
            {
                config.serverAddress = ServerAddress.Text;

                if (ServerPort.Text == string.Empty)
                    config.port = 5154;
                else
                {
                    config.port = int.Parse(ServerPort.Text);
                    if (config.port < 1)
                        config.port = 5154;
                }
                config.publicDescription = PublicDescription.Text;
            }

            config.maxSuperflags = MaxSuperFlags.SelectedIndex * 5;

            config.quitOngame = !ResetOnQuit.Checked;
            config.autoTeam = AutoTeam.Checked;

            config.endType = GameEndType.Never;
            if (GameEnds.SelectedIndex >= 1 && GameEnds.SelectedIndex <= 4)
            {
                config.endType = GameEndType.TimeLimit;
                if (GameEnds.SelectedIndex == 1)
                    config.endValue = 5;
                if (GameEnds.SelectedIndex == 2)
                    config.endValue = 10;
                if (GameEnds.SelectedIndex == 3)
                    config.endValue = 60;
                if (GameEnds.SelectedIndex == 4)
                    config.endValue = 180;
            }
            else if (GameEnds.SelectedIndex >= 5 && GameEnds.SelectedIndex <= 7)
            {
                config.endType = GameEndType.PlayerScoreLimit;
                if (GameEnds.SelectedIndex == 5)
                    config.endValue = 3;
                if (GameEnds.SelectedIndex == 6)
                    config.endValue = 10;
                if (GameEnds.SelectedIndex == 7)
                    config.endValue = 25;
            }
            else if (GameEnds.SelectedIndex >= 8 && GameEnds.SelectedIndex <= 11)
            {
                config.endType = GameEndType.PlayerScoreLimit;
                if (GameEnds.SelectedIndex == 8)
                    config.endValue = 3;
                if (GameEnds.SelectedIndex == 9)
                    config.endValue = 10;
                if (GameEnds.SelectedIndex == 10)
                    config.endValue = 25;
                if (GameEnds.SelectedIndex == 11)
                    config.endValue = 100;
            }
        }

        private void checkAddressItem()
        {
            ServerAddress.Enabled = PublicServer.Checked;
            ServerPort.Enabled = PublicServer.Checked;
            ServerTest.Enabled = PublicServer.Checked;
        }

        private void defaultPrefs ()
        {
            prefs = new Prefrences();
        //    MessageBox.Show("No BZFlag configuration has been found, please be sure to set your directory paths manualy");
        }

        private void savePrefs ()
        {
            // what the hell, try it
            if (!confDir.Exists)
                confDir.Create();
           
            FileInfo confFile = new FileInfo(Path.Combine(confDir.FullName, "StartBZFS.xml"));

            FileStream fs = confFile.OpenWrite();
            if (fs == null)
                return;

            XmlSerializer xml = new XmlSerializer(typeof(Prefrences));
            StreamWriter sr = new StreamWriter(fs);

            xml.Serialize(sr, prefs);
            sr.Close();
            fs.Close();
        }

        private void loadPrefs ( )
        {
            confDir = Prefrences.GetConfigDir();
            if (!confDir.Exists)
            {
                defaultPrefs();
                return;
            }

            FileInfo confFile = new FileInfo(Path.Combine(confDir.FullName, "StartBZFS.xml"));
            if (!confFile.Exists)
                prefs = new Prefrences();
            else
            {
                XmlSerializer xml = new XmlSerializer(typeof(Prefrences));
                FileStream fs = confFile.OpenRead();
                StreamReader sr = new StreamReader(fs);
                prefs = (Prefrences)xml.Deserialize(sr);
                sr.Close();
                fs.Close();
            }
            
            // check for the server dir
            if (prefs.ServerPath == string.Empty || !new FileInfo(prefs.ServerPath).Exists)
            {
                FileInfo server = Prefrences.FindServer(confDir);
                if (server.Exists)
                    prefs.ServerPath = server.FullName;
                else
                    prefs.ServerPath = string.Empty;
            }

            // check for the world dir
            if (prefs.WorldPath == string.Empty || !new DirectoryInfo(prefs.WorldPath).Exists)
            {
                DirectoryInfo worldDir = Prefrences.FindWorldDir(confDir);
                if (worldDir.Exists)
                    prefs.WorldPath = worldDir.FullName;
                else
                    prefs.WorldPath = string.Empty;
            }
        }

        private void checkServerStartButton ()
        {
            bool enable = true;
            Status.Text = "Status:";

            if (prefs.ServerPath == string.Empty || !new FileInfo(prefs.ServerPath).Exists)
            {
                Status.Text += "No server path defined ";
                enable = false;
            }

            if (enable)
                Status.Text += "OK";
            Start.Enabled = enable;
        }

        private void buildWorldsList ( int index )
        {
            WorldsList.Items.Clear();
            WorldsList.Items.Add("Random");
            foreach(FileInfo i in maps)
                WorldsList.Items.Add(Path.GetFileNameWithoutExtension(i.FullName));

           WorldsList.Items.Add("Custom...");
           WorldsList.SelectedIndex = index;
        }

        private void pathsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (new Paths(confDir, prefs).ShowDialog() == DialogResult.OK)
            {
                savePrefs();
                scanMaps();
                buildWorldsList(0);
            }

            checkServerStartButton();
        }

        private void PublicServer_CheckedChanged(object sender, EventArgs e)
        {
            ServerAddress.Enabled = PublicServer.Checked;
            ServerPort.Enabled = PublicServer.Checked;
            ServerTest.Enabled = PublicServer.Checked;
            PublicDescription.Enabled = PublicServer.Checked;
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            savePrefs();
        }

        private void Start_Click(object sender, EventArgs e)
        {
            setConfigFromForm(serverConfig);
            serverConfig.run(RunInBackground.Checked,prefs.ServerPath);
        }

        private void BadFlags_CheckedChanged(object sender, EventArgs e)
        {
            if (BadFlags.Checked)
            {
                if (ShakeTime.Text == string.Empty)
                    ShakeTime.Text = "30";
                if (ShakeWins.Text == string.Empty)
                    ShakeWins.Text = "1";
                Antidote.Checked = true;
            }
        }

        private void CTFMode_CheckedChanged(object sender, EventArgs e)
        {
            if (!FlagsOnBuildings.Checked)
                FlagsOnBuildings.Checked = CTFMode.Checked && Jumping.Checked;
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (settingMaps)
                return;

            Teleporters.Enabled = WorldsList.SelectedIndex == 0;
            RandomRot.Enabled = WorldsList.SelectedIndex == 0;
            RandomHeight.Enabled = WorldsList.SelectedIndex == 0;

            if (WorldsList.SelectedIndex > maps.Count)
            {
                OpenFileDialog ofd = new OpenFileDialog();
                ofd.CheckFileExists = true;
                ofd.Multiselect = false;
                ofd.FileName = "*bzw";
                ofd.InitialDirectory = prefs.WorldPath;
                ofd.Filter = "BZFlag Map files (*.bzw)|*.bzw|All files (*.*)|*.*";

                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    settingMaps = true;
                    maps.Add(new FileInfo(ofd.FileName));
                    buildWorldsList(WorldsList.Items.Count-1);
                    settingMaps = false;
                }
            }
        }

        private void RabbitMode_CheckedChanged(object sender, EventArgs e)
        {
            RabbitModeType.Enabled = RabbitModeItem.Checked;
        }

        private void ServerTest_Click(object sender, EventArgs e)
        {
            ServerTest test = new ServerTest();

            test.address = ServerAddress.Text;
            if (ServerPort.Text != string.Empty)
                test.port = int.Parse(ServerPort.Text);

            test.ShowDialog();
        }

        private void RunInBackground_CheckedChanged(object sender, EventArgs e)
        {
            if (RunInBackground.Checked && !Prefrences.IsWinddows())
                MessageBox.Show("Servers run in background mode must be manual killed on your system");
        }
    }
}
