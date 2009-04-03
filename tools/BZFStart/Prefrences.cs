using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace BZFStart
{
    public class Prefrences
    {
        public static DirectoryInfo GetConfigDir()
        {
            if (Environment.OSVersion.Platform == PlatformID.Unix)
                return new DirectoryInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Personal),".bzf"));
            else if (Environment.OSVersion.Platform == PlatformID.MacOSX)
                return new DirectoryInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "BZFlag"));
            else// windows
                return new DirectoryInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "My BZFlag Files"));
        }

        public static FileInfo FindClient(DirectoryInfo confDir)
        {
            FileInfo client = new FileInfo("\\");

            FileInfo clientDirFile = new FileInfo(Path.Combine(confDir.FullName, "bzflag.dir"));
            if (clientDirFile.Exists)
            {
                StreamReader sr = clientDirFile.OpenText();
                string exePath = sr.ReadLine();
                client = new FileInfo(sr.ReadLine());
                sr.Close();
            }

            return client;
        }

        public static FileInfo FindServer (DirectoryInfo confDir)
        {
            FileInfo server = new FileInfo("\\");

            FileInfo serverDirFile = new FileInfo(Path.Combine(confDir.FullName, "bzfs.dir"));
            if (serverDirFile.Exists)
            {
                StreamReader sr = serverDirFile.OpenText();
                server = new FileInfo(sr.ReadLine());
                sr.Close();
            }

            if (!server.Exists)
            {
                // the server file was bogus, try the client path with "bzfs" on it

                FileInfo clientDirFile = new FileInfo(Path.Combine(confDir.FullName, "bzflag.dir"));
                if (clientDirFile.Exists)
                {
                    string exeFile, exePath;
                    StreamReader sr = clientDirFile.OpenText();
                    exePath = sr.ReadLine();
                    exeFile = sr.ReadLine();
                    sr.Close();

                    server = new FileInfo(Path.Combine(exePath, "bzfs"));
                    if (!server.Exists)
                    {
                        server = new FileInfo(Path.Combine(exePath, "bzfs.exe"));
                        if (!server.Exists)
                        {
                            // ok so we couldn't find it in the stored bzflag dir, so try the exe path
                            exePath = Path.GetDirectoryName(exeFile);
                            server = new FileInfo(Path.Combine(exePath, "bzfs"));
                            if (!server.Exists)
                                server = new FileInfo(Path.Combine(exePath, "bzfs.exe"));
                        }
                    }
                }
            }

            return server;
        }

        public static DirectoryInfo FindWorldDir ( DirectoryInfo confDir )
        {
            DirectoryInfo worldDir = new DirectoryInfo(Path.Combine(confDir.FullName, "worlds"));
            if (!worldDir.Exists)
            {
                worldDir.Create();
                worldDir = new DirectoryInfo(worldDir.FullName);
            }
            return worldDir;
        }

        public string ServerPath = string.Empty;
        public string ClientPath = string.Empty;
        public string WorldPath = string.Empty;

        public bool LocatePaths ( DirectoryInfo configDir )
        {
            return false;
        }
    }
}
