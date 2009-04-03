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

    public class LogScaner
    {
        public StreamReader reader;

        List<string> lines = new List<string>();

        object nugget = new object();

        public string popLine ()
        {
            lock(nugget)
            {
                if (lines.Count > 0)
                {
                    string line = lines[0];
                    lines.RemoveAt(0);
                    return line;
                }
            }
            return string.Empty;
        }

        public LogScaner(StreamReader r)
        {
            reader = r;
        }

        public void read ()
        {
            while(!reader.EndOfStream)
            {
 //               int read = reader.Read();
                string line = reader.ReadLine();
                lock(nugget)
                {
                    lines.Add(line);
                }
            }
        }
    }

    public class ServerLoger
    {
        public Process serverProc = new Process();

        ServerConfig config;

        ServerLogWindow logWindow;

        public ServerLoger(ServerConfig cfg)
        {
            config = cfg;
        }

        public void run ( string command, string args, string tempLogiFile )
        {
            logWindow = new ServerLogWindow();
            logWindow.Show();

            serverProc.StartInfo.Arguments = args;// +redirectCommand(tempLogiFile);
            serverProc.StartInfo.FileName = command;
            serverProc.StartInfo.WorkingDirectory = Path.GetDirectoryName(command);

            serverProc.StartInfo.RedirectStandardError = true;
            serverProc.StartInfo.RedirectStandardOutput = true;
            serverProc.StartInfo.UseShellExecute = false;
            serverProc.StartInfo.CreateNoWindow = true;
            serverProc.Start();

            LogScaner logWriterStd = new LogScaner(serverProc.StandardOutput);
            LogScaner logWriterErr = new LogScaner(serverProc.StandardError);

            logWindow.addText("Server Started");
           // logWindow.addText("Loging dosn't work yet, so deal");

            Thread stdOutThread = new Thread(new ThreadStart(logWriterStd.read));
            Thread stdErrThread = new Thread(new ThreadStart(logWriterErr.read));

            stdOutThread.Start();
            stdErrThread.Start();

            while (!serverProc.HasExited)
            {
                if (logWindow.stop)
                    serverProc.Kill();
                else
                    Thread.Sleep(100);

                string line = logWriterStd.popLine();
                if (line != string.Empty)
                    logWindow.addText(line);

                line = logWriterErr.popLine();
                if (line != string.Empty)
                    logWindow.addText("Error: " + line);
                
                // just so we can do this single threaded for debugin
                Application.DoEvents();
            }
            stdOutThread.Abort();
            stdErrThread.Abort();
            //  logWindow.addText("Server Died man.. sorry);

            logWindow.Dispose();
            config.ServerTerminate(this);
        }
    }

    public class ServerConfig
    {
        public GameMode mode = GameMode.FFA;
        public bool publicServer = false;

        public string serverAddress = "localhost";
        public int port = 5154;

        List<ServerLoger> logers = new List<ServerLoger>();
        bool killing = false;

        public void ServerTerminate ( ServerLoger loger )
        {
            if (!killing)
                logers.Remove(loger);
        }

        public void buildConfig(FileStream stream)
        {
            StreamWriter sw = new StreamWriter(stream);

            switch(mode)
            {
                case GameMode.CTF:
                    sw.WriteLine("-c");
                    break;

                case GameMode.OpenFFA:
                    sw.Write("-offa");
                    break;

                case GameMode.Rabbit:
                    sw.Write("-rabbit");
                    break;
            }

            sw.WriteLine("-d");
            sw.WriteLine("-d");
            sw.WriteLine("-d");
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
