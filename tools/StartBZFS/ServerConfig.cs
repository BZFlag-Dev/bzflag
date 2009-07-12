using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace StartBZFS
{
    public enum GameMode
    {
        FFA,
        OpenFFA,
        CTF,
        Rabbit
    }

    public enum RabbitMode
    {
        ScoreBased = 0,
        KillerBased = 1,
        Random = 2
    }

    public enum GameEndType
    {
        Never,
        TimeLimit,
        PlayerScoreLimit,
        TeamScoreLimit
    }

    public class ServerConfig
    {
        public GameMode mode = GameMode.FFA;
        public RabbitMode rabbitMode = RabbitMode.ScoreBased;

        public string password = string.Empty;

        public GameEndType endType = GameEndType.Never;
        public int endValue = -1;

        public bool handicap = false;

        public int maxPlayers = -1;

        public int maxSuperflags = -1;

        public bool publicServer = false;

        public string publicDescription = string.Empty;

        public string serverAddress = "localhost";
        public int port = 5154;

        public int shots = 1;
        public int debugLevel = 0;

        public bool goodFlags = false;
        public bool badFlags = false;
        public bool flagsOnBuildings = false;

        public bool jumping = true;
        public bool rico = true;

        public int shakeTime = 0;
        public int shakeWins = 0;
        public bool andidote = true;

        public string worldfile = string.Empty;
        public bool teleporters = true;
        public bool randHeight = true;
        public bool randRot = true;
        public bool spawnOnBoxes = false;

        public bool quitOngame = false;

        public bool autoTeam = true;

        [System.Xml.Serialization.XmlIgnoreAttribute]
        List<ServerLoger> logers = new List<ServerLoger>();

        [System.Xml.Serialization.XmlIgnoreAttribute]
        bool killing = false;

        public void ServerTerminate ( ServerLoger loger )
        {
            if (!killing)
                logers.Remove(loger);
        }

        public void buildConfig(FileStream stream)
        {
            StreamWriter sw = new StreamWriter(stream);

            if (rico)
                sw.WriteLine("+r");

            if (password != string.Empty)
                sw.WriteLine("-password " + password);

            if (autoTeam)
                sw.WriteLine("-autoTeam");

            if (jumping)
                sw.WriteLine("-j");

            if (spawnOnBoxes)
                sw.WriteLine("-sb");

            if (maxSuperflags > 0)
                sw.WriteLine("+s " + maxSuperflags.ToString());

            if (handicap)
                sw.WriteLine("-h");

            if (maxPlayers > 0)
                sw.WriteLine("-mp " + maxPlayers.ToString());

            switch(mode)
            {
                case GameMode.CTF:
                    if (worldfile != string.Empty)
                        sw.WriteLine("-c");
                    else
                        sw.WriteLine("-cr");
                    break;

                case GameMode.OpenFFA:
                    sw.WriteLine("-offa");
                    break;

                case GameMode.Rabbit:
                    sw.Write("-rabbit ");
                    switch(rabbitMode)
                    {
                        case RabbitMode.ScoreBased:
                            sw.WriteLine("score");
                            break;
                        case RabbitMode.KillerBased:
                            sw.WriteLine("killer");
                            break;
                        case RabbitMode.Random:
                            sw.WriteLine("random");
                            break;
                  }
                    break;
            }

            if (goodFlags)
                sw.WriteLine("+f good");

            if (badFlags)
                sw.WriteLine("+f bad");

            if (shakeWins > 0)
                sw.WriteLine("-sw " + shakeWins.ToString());

            if (shakeTime > 0)
                sw.WriteLine("-st " + shakeTime.ToString());

            if (shots > 1)
                sw.WriteLine("-ms " + shots.ToString());

            if (flagsOnBuildings)
                sw.WriteLine("-fb");

            if (andidote)
                sw.WriteLine("-sa");

            if (quitOngame)
                sw.WriteLine("-g");

            if (endType != GameEndType.Never)
            {
                if (endType == GameEndType.PlayerScoreLimit)
                    sw.WriteLine("-mps " + endValue.ToString());
                else if (endType == GameEndType.TeamScoreLimit)
                    sw.WriteLine("-mts " + endValue.ToString());
                else if (endType == GameEndType.TimeLimit)
                    sw.WriteLine("-time " + (endValue*60).ToString());
            }

            if (publicServer)
            {
                sw.Write("-public");
                if (publicDescription != string.Empty)
                {
                    sw.WriteLine(" \"" + publicDescription.Replace('\"',' ') + "\"");

                    if (port > 0 && port != 5154)
                        sw.WriteLine("-p " + port.ToString());

                    if (serverAddress != string.Empty)
                    {
                        sw.Write("-publicaddr " + serverAddress);
                         if (port > 0 && port != 5154)
                              sw.Write(":" + port.ToString());
                         sw.WriteLine("");
                    }
                }
            }

            if (debugLevel > 0)
            {
                for(int i = 0; i < debugLevel; i++)
                    sw.WriteLine("-d");
            }

            // write the world last so its' config options overide this
            if (worldfile != string.Empty)
                sw.WriteLine("-world \"" + worldfile + "\"");
            else
            {
                if (teleporters)
                    sw.WriteLine("-t");

                if (randHeight)
                    sw.WriteLine("-h");

                if (randRot)
                    sw.WriteLine("-b");
            }
            sw.Close();
        }

        public void killAll ( )
        {
            killing = true;
            foreach (ServerLoger l in logers)
                l.serverProc.Kill();

            killing = false;
        }

        public void run ( bool background, string exePath )
        {
            DirectoryInfo configDir = Prefrences.GetConfigDir();
            if (!configDir.Exists)
                configDir.Create();
            DirectoryInfo tempDir = configDir.CreateSubdirectory("temp");
            if (tempDir == null || !tempDir.Exists)
            {
                MessageBox.Show("Unable to start server, error creating temp dir\n" + tempDir.FullName);
                return;
            }

            if (port < 1)
                port = 5154;

            FileInfo configFile = new FileInfo(Path.Combine(tempDir.FullName, port.ToString()+".cfg"));

            FileStream fs = configFile.Create();
            if (fs == null)
            {
                MessageBox.Show("Unable to start server, error creating config file\n" + configFile.FullName);
                return;
            }
            buildConfig(fs);
            fs.Close();

            string commandLine = "-conf \"" + configFile.FullName + "\"";

            if (!background)
            {
                ServerLoger loger = new ServerLoger(this);
                logers.Add(loger);

                // throw this in a thread, for reals but for now just leave it single threaded for debugin
                loger.run(exePath, commandLine, Path.Combine(tempDir.FullName, port.ToString() + ".log"));
            }
            else
            {
                Process proc = new Process();
                proc.StartInfo.Arguments = commandLine;
                proc.StartInfo.FileName = exePath;
                proc.StartInfo.WorkingDirectory = Path.GetDirectoryName(exePath);
                proc.Start();
            }
        }
    }
}
