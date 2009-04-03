using System;
using System.Collections.Generic;
using System.Linq;
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

    public class ServerConfig
    {
        public GameMode mode = GameMode.FFA;
        public bool publicServer = false;

        public string serverAddress = "localhost";
        public int port = 5154;

        public int shots = 1;
        public int debugLevel = 3;

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
        public bool spawnOnBoxes = false;

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

            if (worldfile != string.Empty)
                sw.WriteLine("-world \"" + worldfile + "\"");
            else if (teleporters)
                sw.WriteLine("-t");

            if (jumping)
                sw.WriteLine("-j");

            if (spawnOnBoxes)
                sw.WriteLine("-sb");

            switch(mode)
            {
                case GameMode.CTF:
                    sw.WriteLine("-c");
                    break;

                case GameMode.OpenFFA:
                    sw.WriteLine("-offa");
                    break;

                case GameMode.Rabbit:
                    sw.WriteLine("-rabbit");
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

            if (debugLevel > 0)
            {
                for(int i = 0; i < debugLevel; i++)
                    sw.WriteLine("-d");
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
