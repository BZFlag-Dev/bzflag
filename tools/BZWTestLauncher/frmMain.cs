using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

using System.IO;
using System.Diagnostics;
using System.Threading;
using System.Security.Permissions;

using Microsoft.Win32;

namespace BZWTestLauncher
{
	/// <summary>
	/// Summary description for frmMain.
	/// </summary>
	public class frmMain : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtBZFSBinary;
		private System.Windows.Forms.Button btnBZFSBinaryBrowse;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button btnBZFlagBinaryBrowse;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Button btnBZWFileBrowse;
		private System.Windows.Forms.TextBox txtBZWFile;
		private System.Windows.Forms.TextBox txtBZFlagBinary;
		private System.Windows.Forms.GroupBox grpGameMode;
		private System.Windows.Forms.RadioButton rdoGameModeFFA;
		private System.Windows.Forms.RadioButton rdoGameModeCTF;
		private System.Windows.Forms.RadioButton rdoGameModeRabbit;
		private System.Windows.Forms.GroupBox grpMiscSettings;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.NumericUpDown nudDebugLevel;
		private System.Windows.Forms.CheckBox chkJumping;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.NumericUpDown nudMaxShots;
		private System.Windows.Forms.CheckBox chkRicochet;
		private System.Windows.Forms.CheckBox chkTanksSpawnOnBuildings;
		private System.Windows.Forms.GroupBox grpClientSettings;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.NumericUpDown nudSoloBots;
		private System.Windows.Forms.Button btnRun;
		private System.Windows.Forms.Button btnStop;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		private Process procBZFS;
		private BZFSPoller outPoll;
		private BZFSPoller errPoll;
		private System.Windows.Forms.CheckBox chkRunClient;
		private System.Windows.Forms.RichTextBox txtOutput;
		private Process procBZFlag;

		public frmMain()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			procBZFS = new Process();
			procBZFlag = new Process();

			LoadSettings();

			txtOutput.Text = "Run the map, and then press Stop to view BZFS output, if any.";
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				bzfsStop();
				bzflagStop();
				SaveSettings();

				if (components != null) 
				{
					components.Dispose();
				}

				BZFSPoller.Exit = true;
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.label1 = new System.Windows.Forms.Label();
			this.txtBZFSBinary = new System.Windows.Forms.TextBox();
			this.btnBZFSBinaryBrowse = new System.Windows.Forms.Button();
			this.txtBZFlagBinary = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.btnBZFlagBinaryBrowse = new System.Windows.Forms.Button();
			this.btnBZWFileBrowse = new System.Windows.Forms.Button();
			this.txtBZWFile = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.grpGameMode = new System.Windows.Forms.GroupBox();
			this.rdoGameModeFFA = new System.Windows.Forms.RadioButton();
			this.rdoGameModeCTF = new System.Windows.Forms.RadioButton();
			this.rdoGameModeRabbit = new System.Windows.Forms.RadioButton();
			this.grpMiscSettings = new System.Windows.Forms.GroupBox();
			this.chkJumping = new System.Windows.Forms.CheckBox();
			this.nudDebugLevel = new System.Windows.Forms.NumericUpDown();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.nudMaxShots = new System.Windows.Forms.NumericUpDown();
			this.chkRicochet = new System.Windows.Forms.CheckBox();
			this.chkTanksSpawnOnBuildings = new System.Windows.Forms.CheckBox();
			this.grpClientSettings = new System.Windows.Forms.GroupBox();
			this.chkRunClient = new System.Windows.Forms.CheckBox();
			this.nudSoloBots = new System.Windows.Forms.NumericUpDown();
			this.label6 = new System.Windows.Forms.Label();
			this.btnRun = new System.Windows.Forms.Button();
			this.btnStop = new System.Windows.Forms.Button();
			this.txtOutput = new System.Windows.Forms.RichTextBox();
			this.grpGameMode.SuspendLayout();
			this.grpMiscSettings.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.nudDebugLevel)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nudMaxShots)).BeginInit();
			this.grpClientSettings.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.nudSoloBots)).BeginInit();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(184, 16);
			this.label1.TabIndex = 0;
			this.label1.Text = "BZFS.exe Binary:";
			// 
			// txtBZFSBinary
			// 
			this.txtBZFSBinary.Location = new System.Drawing.Point(8, 24);
			this.txtBZFSBinary.Name = "txtBZFSBinary";
			this.txtBZFSBinary.Size = new System.Drawing.Size(248, 20);
			this.txtBZFSBinary.TabIndex = 1;
			this.txtBZFSBinary.Text = "";
			// 
			// btnBZFSBinaryBrowse
			// 
			this.btnBZFSBinaryBrowse.Location = new System.Drawing.Point(264, 24);
			this.btnBZFSBinaryBrowse.Name = "btnBZFSBinaryBrowse";
			this.btnBZFSBinaryBrowse.Size = new System.Drawing.Size(80, 24);
			this.btnBZFSBinaryBrowse.TabIndex = 2;
			this.btnBZFSBinaryBrowse.Text = "Browse...";
			this.btnBZFSBinaryBrowse.Click += new System.EventHandler(this.btnBZFSBinaryBrowse_Click);
			// 
			// txtBZFlagBinary
			// 
			this.txtBZFlagBinary.Location = new System.Drawing.Point(8, 64);
			this.txtBZFlagBinary.Name = "txtBZFlagBinary";
			this.txtBZFlagBinary.Size = new System.Drawing.Size(248, 20);
			this.txtBZFlagBinary.TabIndex = 1;
			this.txtBZFlagBinary.Text = "";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(8, 48);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(184, 16);
			this.label2.TabIndex = 0;
			this.label2.Text = "BZFlag.exe Binary:";
			// 
			// btnBZFlagBinaryBrowse
			// 
			this.btnBZFlagBinaryBrowse.Location = new System.Drawing.Point(264, 64);
			this.btnBZFlagBinaryBrowse.Name = "btnBZFlagBinaryBrowse";
			this.btnBZFlagBinaryBrowse.Size = new System.Drawing.Size(80, 24);
			this.btnBZFlagBinaryBrowse.TabIndex = 2;
			this.btnBZFlagBinaryBrowse.Text = "Browse...";
			this.btnBZFlagBinaryBrowse.Click += new System.EventHandler(this.btnBZFlagBinaryBrowse_Click);
			// 
			// btnBZWFileBrowse
			// 
			this.btnBZWFileBrowse.Location = new System.Drawing.Point(264, 104);
			this.btnBZWFileBrowse.Name = "btnBZWFileBrowse";
			this.btnBZWFileBrowse.Size = new System.Drawing.Size(80, 24);
			this.btnBZWFileBrowse.TabIndex = 2;
			this.btnBZWFileBrowse.Text = "Browse...";
			this.btnBZWFileBrowse.Click += new System.EventHandler(this.btnBZWFileBrowse_Click);
			// 
			// txtBZWFile
			// 
			this.txtBZWFile.Location = new System.Drawing.Point(8, 104);
			this.txtBZWFile.Name = "txtBZWFile";
			this.txtBZWFile.Size = new System.Drawing.Size(248, 20);
			this.txtBZWFile.TabIndex = 1;
			this.txtBZWFile.Text = "";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(8, 88);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(184, 16);
			this.label3.TabIndex = 0;
			this.label3.Text = "BZW World File:";
			// 
			// grpGameMode
			// 
			this.grpGameMode.Controls.Add(this.rdoGameModeFFA);
			this.grpGameMode.Controls.Add(this.rdoGameModeCTF);
			this.grpGameMode.Controls.Add(this.rdoGameModeRabbit);
			this.grpGameMode.Location = new System.Drawing.Point(8, 136);
			this.grpGameMode.Name = "grpGameMode";
			this.grpGameMode.Size = new System.Drawing.Size(120, 56);
			this.grpGameMode.TabIndex = 3;
			this.grpGameMode.TabStop = false;
			this.grpGameMode.Text = "Game Mode";
			// 
			// rdoGameModeFFA
			// 
			this.rdoGameModeFFA.Location = new System.Drawing.Point(8, 16);
			this.rdoGameModeFFA.Name = "rdoGameModeFFA";
			this.rdoGameModeFFA.Size = new System.Drawing.Size(48, 16);
			this.rdoGameModeFFA.TabIndex = 1;
			this.rdoGameModeFFA.Text = "FFA";
			// 
			// rdoGameModeCTF
			// 
			this.rdoGameModeCTF.Location = new System.Drawing.Point(64, 16);
			this.rdoGameModeCTF.Name = "rdoGameModeCTF";
			this.rdoGameModeCTF.Size = new System.Drawing.Size(48, 16);
			this.rdoGameModeCTF.TabIndex = 1;
			this.rdoGameModeCTF.Text = "CTF";
			// 
			// rdoGameModeRabbit
			// 
			this.rdoGameModeRabbit.Location = new System.Drawing.Point(8, 32);
			this.rdoGameModeRabbit.Name = "rdoGameModeRabbit";
			this.rdoGameModeRabbit.Size = new System.Drawing.Size(56, 16);
			this.rdoGameModeRabbit.TabIndex = 1;
			this.rdoGameModeRabbit.Text = "Rabbit";
			// 
			// grpMiscSettings
			// 
			this.grpMiscSettings.Controls.Add(this.chkJumping);
			this.grpMiscSettings.Controls.Add(this.nudDebugLevel);
			this.grpMiscSettings.Controls.Add(this.label4);
			this.grpMiscSettings.Controls.Add(this.label5);
			this.grpMiscSettings.Controls.Add(this.nudMaxShots);
			this.grpMiscSettings.Controls.Add(this.chkRicochet);
			this.grpMiscSettings.Controls.Add(this.chkTanksSpawnOnBuildings);
			this.grpMiscSettings.Location = new System.Drawing.Point(136, 136);
			this.grpMiscSettings.Name = "grpMiscSettings";
			this.grpMiscSettings.Size = new System.Drawing.Size(208, 104);
			this.grpMiscSettings.TabIndex = 4;
			this.grpMiscSettings.TabStop = false;
			this.grpMiscSettings.Text = "Misc Settings";
			// 
			// chkJumping
			// 
			this.chkJumping.Location = new System.Drawing.Point(128, 16);
			this.chkJumping.Name = "chkJumping";
			this.chkJumping.Size = new System.Drawing.Size(72, 16);
			this.chkJumping.TabIndex = 2;
			this.chkJumping.Text = "Jumping";
			// 
			// nudDebugLevel
			// 
			this.nudDebugLevel.Location = new System.Drawing.Point(80, 16);
			this.nudDebugLevel.Maximum = new System.Decimal(new int[] {
																		  4,
																		  0,
																		  0,
																		  0});
			this.nudDebugLevel.Name = "nudDebugLevel";
			this.nudDebugLevel.Size = new System.Drawing.Size(40, 20);
			this.nudDebugLevel.TabIndex = 1;
			this.nudDebugLevel.Value = new System.Decimal(new int[] {
																		1,
																		0,
																		0,
																		0});
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(8, 16);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(72, 16);
			this.label4.TabIndex = 0;
			this.label4.Text = "Debug Level:";
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(8, 40);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(72, 16);
			this.label5.TabIndex = 0;
			this.label5.Text = "Max Shots:";
			// 
			// nudMaxShots
			// 
			this.nudMaxShots.Location = new System.Drawing.Point(80, 40);
			this.nudMaxShots.Maximum = new System.Decimal(new int[] {
																		20,
																		0,
																		0,
																		0});
			this.nudMaxShots.Name = "nudMaxShots";
			this.nudMaxShots.Size = new System.Drawing.Size(40, 20);
			this.nudMaxShots.TabIndex = 1;
			this.nudMaxShots.Value = new System.Decimal(new int[] {
																	  5,
																	  0,
																	  0,
																	  0});
			// 
			// chkRicochet
			// 
			this.chkRicochet.Location = new System.Drawing.Point(128, 40);
			this.chkRicochet.Name = "chkRicochet";
			this.chkRicochet.Size = new System.Drawing.Size(72, 16);
			this.chkRicochet.TabIndex = 2;
			this.chkRicochet.Text = "Ricochet";
			// 
			// chkTanksSpawnOnBuildings
			// 
			this.chkTanksSpawnOnBuildings.Location = new System.Drawing.Point(8, 64);
			this.chkTanksSpawnOnBuildings.Name = "chkTanksSpawnOnBuildings";
			this.chkTanksSpawnOnBuildings.Size = new System.Drawing.Size(96, 32);
			this.chkTanksSpawnOnBuildings.TabIndex = 2;
			this.chkTanksSpawnOnBuildings.Text = "Tanks Spawn On Buildings";
			// 
			// grpClientSettings
			// 
			this.grpClientSettings.Controls.Add(this.chkRunClient);
			this.grpClientSettings.Controls.Add(this.nudSoloBots);
			this.grpClientSettings.Controls.Add(this.label6);
			this.grpClientSettings.Location = new System.Drawing.Point(8, 200);
			this.grpClientSettings.Name = "grpClientSettings";
			this.grpClientSettings.Size = new System.Drawing.Size(120, 72);
			this.grpClientSettings.TabIndex = 5;
			this.grpClientSettings.TabStop = false;
			this.grpClientSettings.Text = "Client Settings";
			// 
			// chkRunClient
			// 
			this.chkRunClient.Checked = true;
			this.chkRunClient.CheckState = System.Windows.Forms.CheckState.Checked;
			this.chkRunClient.Location = new System.Drawing.Point(8, 16);
			this.chkRunClient.Name = "chkRunClient";
			this.chkRunClient.Size = new System.Drawing.Size(80, 16);
			this.chkRunClient.TabIndex = 2;
			this.chkRunClient.Text = "Run Client";
			// 
			// nudSoloBots
			// 
			this.nudSoloBots.Location = new System.Drawing.Point(40, 40);
			this.nudSoloBots.Maximum = new System.Decimal(new int[] {
																		10,
																		0,
																		0,
																		0});
			this.nudSoloBots.Name = "nudSoloBots";
			this.nudSoloBots.Size = new System.Drawing.Size(40, 20);
			this.nudSoloBots.TabIndex = 1;
			// 
			// label6
			// 
			this.label6.Location = new System.Drawing.Point(8, 40);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(32, 16);
			this.label6.TabIndex = 0;
			this.label6.Text = "Bots:";
			// 
			// btnRun
			// 
			this.btnRun.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.btnRun.Location = new System.Drawing.Point(136, 248);
			this.btnRun.Name = "btnRun";
			this.btnRun.Size = new System.Drawing.Size(104, 40);
			this.btnRun.TabIndex = 7;
			this.btnRun.Text = "Run...";
			this.btnRun.Click += new System.EventHandler(this.btnRun_Click);
			// 
			// btnStop
			// 
			this.btnStop.Enabled = false;
			this.btnStop.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.btnStop.Location = new System.Drawing.Point(240, 248);
			this.btnStop.Name = "btnStop";
			this.btnStop.Size = new System.Drawing.Size(104, 40);
			this.btnStop.TabIndex = 7;
			this.btnStop.Text = "Stop...";
			this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
			// 
			// txtOutput
			// 
			this.txtOutput.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.txtOutput.Location = new System.Drawing.Point(352, 8);
			this.txtOutput.Name = "txtOutput";
			this.txtOutput.ReadOnly = true;
			this.txtOutput.Size = new System.Drawing.Size(392, 280);
			this.txtOutput.TabIndex = 8;
			this.txtOutput.Text = "richTextBox1";
			// 
			// frmMain
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(752, 296);
			this.Controls.Add(this.txtOutput);
			this.Controls.Add(this.btnRun);
			this.Controls.Add(this.grpClientSettings);
			this.Controls.Add(this.grpMiscSettings);
			this.Controls.Add(this.grpGameMode);
			this.Controls.Add(this.btnBZFSBinaryBrowse);
			this.Controls.Add(this.txtBZFSBinary);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.txtBZFlagBinary);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.btnBZFlagBinaryBrowse);
			this.Controls.Add(this.btnBZWFileBrowse);
			this.Controls.Add(this.txtBZWFile);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.btnStop);
			this.Name = "frmMain";
			this.Text = "BZW Test Launcher";
			this.grpGameMode.ResumeLayout(false);
			this.grpMiscSettings.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.nudDebugLevel)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nudMaxShots)).EndInit();
			this.grpClientSettings.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.nudSoloBots)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new frmMain());
		}

		private void LoadSettings()
		{
			RegistryKey root = Registry.CurrentUser.OpenSubKey(@"Software\"+Application.ProductName);
			if (root == null) return;
			
			txtBZFSBinary.Text = (string)root.GetValue("BZFSBinary", "");
			txtBZFlagBinary.Text = (string)root.GetValue("BZFlagBinary", "");
			txtBZWFile.Text = (string)root.GetValue("BZWFile", "");

			switch ((int)root.GetValue("GameMode", 1))
			{
				case 1:
					rdoGameModeFFA.Checked = true;
					break;
				case 2:
					rdoGameModeCTF.Checked = true;
					break;
				case 3:
					rdoGameModeRabbit.Checked = true;
					break;
				default:
					rdoGameModeFFA.Checked = true;
					break;
			}

			chkJumping.Checked = ((int)root.GetValue("Jumping", 1) == 1)?true:false;
			chkRicochet.Checked = ((int)root.GetValue("Ricochet", 1) == 1)?true:false;
			chkTanksSpawnOnBuildings.Checked = ((int)root.GetValue("TanksSpawnOnBuildings", 1) == 1)?true:false;

			nudMaxShots.Value = (int)root.GetValue("MaxShots", 5);
			nudDebugLevel.Value = (int)root.GetValue("DebugLevel", 1);
			nudSoloBots.Value = (int)root.GetValue("SoloBots", 0);
		}

		private void SaveSettings()
		{
			RegistryKey root = Registry.CurrentUser.OpenSubKey(@"Software\"+Application.ProductName, true);
			if (root == null) root = Registry.CurrentUser.CreateSubKey(@"Software\"+Application.ProductName);
			
			root.SetValue("BZFSBinary", txtBZFSBinary.Text);
			root.SetValue("BZFlagBinary", txtBZFlagBinary.Text);
			root.SetValue("BZWFile", txtBZWFile.Text);

			if (rdoGameModeFFA.Checked)
				root.SetValue("GameMode", 1);
			else if (rdoGameModeCTF.Checked)
				root.SetValue("GameMode", 2);
			else if (rdoGameModeRabbit.Checked)
				root.SetValue("GameMode", 3);

			root.SetValue("Jumping", (chkJumping.Checked)?1:0);
			root.SetValue("Ricochet", (chkRicochet.Checked)?1:0);
			root.SetValue("TanksSpawnOnBuildings", (chkTanksSpawnOnBuildings.Checked)?1:0);

			root.SetValue("MaxShots", (int)nudMaxShots.Value);
			root.SetValue("DebugLevel", (int)nudDebugLevel.Value);
			root.SetValue("SoloBots", (int)nudSoloBots.Value);
		}

		private void btnBZFSBinaryBrowse_Click(object sender, System.EventArgs e)
		{
			OpenFileDialog dialog = new OpenFileDialog();
			dialog.Title = "BZFS.exe Binary";
			dialog.Filter = "bzfs executable|bzfs.exe;bzfs|All Files|*";
			dialog.FilterIndex = 1;
			if (txtBZFSBinary.Text.Length == 0)
				dialog.InitialDirectory = System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFiles);
			else
				dialog.InitialDirectory = Path.GetDirectoryName(txtBZFSBinary.Text);

			if (dialog.ShowDialog() == DialogResult.OK)
			{
				txtBZFSBinary.Text = dialog.FileName;
			}
		}

		private void btnBZFlagBinaryBrowse_Click(object sender, System.EventArgs e)
		{
			OpenFileDialog dialog = new OpenFileDialog();
			dialog.Title = "BZFlag.exe Binary";
			dialog.Filter = "bzflag executable|bzflag.exe;bzflag|All Files|*";
			dialog.FilterIndex = 1;
			if (txtBZFlagBinary.Text.Length == 0)
				dialog.InitialDirectory = System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFiles);
			else
				dialog.InitialDirectory = Path.GetDirectoryName(txtBZFlagBinary.Text);


			if (dialog.ShowDialog() == DialogResult.OK)
			{
				txtBZFlagBinary.Text = dialog.FileName;
			}
		}

		private void btnBZWFileBrowse_Click(object sender, System.EventArgs e)
		{
			OpenFileDialog dialog = new OpenFileDialog();
			dialog.Title = "BZFlag World File";
			dialog.Filter = "BZFlag World File (*.bzw)|*.bzw|All files (*.*)|*.*";
			dialog.FilterIndex = 1;
			if (txtBZWFile.Text.Length == 0)
				dialog.InitialDirectory = System.Environment.GetFolderPath(System.Environment.SpecialFolder.Desktop);
			else
				dialog.InitialDirectory = Path.GetDirectoryName(txtBZWFile.Text);

			if (dialog.ShowDialog() == DialogResult.OK)
			{
				txtBZWFile.Text = dialog.FileName;
			}
		}

		private void btnRun_Click(object sender, System.EventArgs e)
		{
			if (!bzfsStart())
			{
				return;
			}

			if (chkRunClient.Checked)
			{
				// Give BZFS a chance to start up
				for (int i = 0; i < 20; ++i)
				{
					Thread.Sleep(100);
					Application.DoEvents();
				}

				if (!bzflagStart())
				{
					bzfsStop();
					return;
				}
			}

			btnRun.Enabled = false;
			btnStop.Enabled = true;
		}

		private void btnStop_Click(object sender, System.EventArgs e)
		{
			bzfsStop();
			bzflagStop();

			btnRun.Enabled = true;
			btnStop.Enabled = false;
		}

		private bool bzfsStart()
		{
			// Step 1) Verify that the files exist
			if (!File.Exists(txtBZFSBinary.Text))
			{
				MessageBox.Show("The BZFS binary was not valid.", "BZFS Binary Not Found", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return false;
			}
			else if (!File.Exists(txtBZWFile.Text))
			{
				MessageBox.Show("The BZW file was not valid.", "BZW File Not Found", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return false;
			}

			// Step 2) Make sure we can find the temporary directory for BZWTL
			string tmp = Path.GetTempPath();

			if (!Directory.Exists(tmp))
			{
				MessageBox.Show("Error locating temporary directory:\n"+tmp, "Temporary Directory");
				return false;
			}
			else if (!Directory.Exists(tmp+@"bzwtl\")) 
			{
				DirectoryInfo dir = new DirectoryInfo(tmp);
				try
				{
					dir.CreateSubdirectory("bzwtl");
				}
				catch (IOException e)
				{
					MessageBox.Show(e.Message, "Error creating temporary directory");
					return false;
				}
			}

			tmp = tmp+@"bzwtl\";

			// Step 3) Create our temporary files (groupdb, etc)
			FileInfo figroupdb = new FileInfo(tmp+"groupdb");
			figroupdb.Delete();
			StreamWriter swgroupdb = figroupdb.CreateText();
			swgroupdb.WriteLine("EVERYONE: actionMessage date endGame flagHistory flagMaster flagMod idleStats kick kill lagStats privateMessage rejoin say setVar shutdownServer spawn superKill talk");
			swgroupdb.Close();

			// Step 4) Generate arguments
			string args = "";

			// Game Mode
			if (rdoGameModeCTF.Checked)
				args += "-c ";
			else if (rdoGameModeRabbit.Checked)
				args += "-rabbit ";

			// Max shots
			args += "-ms "+nudMaxShots.Value.ToString()+" ";

			// Jumping
			if (chkJumping.Checked)
				args += "-j ";

			// Ricochet
			if (chkRicochet.Checked)
				args += "+r ";

			// Tanks Spawn On Buildings
			if (chkTanksSpawnOnBuildings.Checked)
				args += "-sb ";

			// Debug level
			if (nudDebugLevel.Value > 0)
			{
				args += "-";
				for (int i = 0; i < nudDebugLevel.Value; i++)
					args += "d";
				args += " ";
			}
			
			// Always on
			args += "-noMasterBanlist -st 5 -sw 1 ";

			// Group file
			args += "-groupdb \""+tmp+"groupdb\" ";

			// World file
			args += "-world \""+txtBZWFile.Text+"\"";


			// Step 5) Run bzfs
			procBZFS.StartInfo.FileName = txtBZFSBinary.Text;
			procBZFS.StartInfo.Arguments = args;
			procBZFS.StartInfo.UseShellExecute = false;
			procBZFS.StartInfo.CreateNoWindow = true;
			procBZFS.StartInfo.RedirectStandardError = true;
			procBZFS.StartInfo.RedirectStandardOutput = true;
			procBZFS.EnableRaisingEvents = true;
			procBZFS.Exited += new EventHandler(procBZFS_Exited);

			if (!procBZFS.Start())
			{
				MessageBox.Show("Failed to start BZFS.", "Failed to start BZFS");
				return false;
			}

			txtOutput.Rtf = "{\\rtf1\\ansi }";

			BZFSPoller.OutputBox = txtOutput;
			outPoll = new BZFSPoller(procBZFS.StandardOutput, false);
			errPoll = new BZFSPoller(procBZFS.StandardError, true);

			Thread thrdBZFSOut = new Thread(new ThreadStart(outPoll.pollBZFS));
			Thread thrdBZFSErr = new Thread(new ThreadStart(errPoll.pollBZFS));
			Thread thrdCollate = new Thread(new ThreadStart(BZFSPoller.collator));

			thrdBZFSErr.Start();
			thrdBZFSOut.Start();
			thrdCollate.Start();

			return true;
		}

		private bool bzfsStop()
		{
			if (procBZFS.StartInfo.FileName.Length != 0 && !procBZFS.HasExited)
			{
				procBZFS.Kill();
			}
			BZFSPoller.Exit = true;
			return true;
		}

		private bool bzflagStart()
		{
			// Step 1) Verify that the file exist
			if (!File.Exists(txtBZFlagBinary.Text))
			{
				MessageBox.Show("The BZFlag binary was not valid.", "BZFlag Binary Not Found", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return false;
			}

			// Step 2) Set up the arguments
			string args = "";
			if (nudSoloBots.Value > 0)
				args += "-solo "+nudSoloBots.Value.ToString()+" ";

			args += "localhost:5154";

			// Step 2) Lauch bzflag
			procBZFlag.StartInfo.FileName = txtBZFlagBinary.Text;
			procBZFlag.StartInfo.WorkingDirectory = Path.GetDirectoryName(txtBZFlagBinary.Text);
			procBZFlag.StartInfo.Arguments = args;
			procBZFlag.StartInfo.UseShellExecute = false;

			if (!procBZFlag.Start())
			{
				MessageBox.Show("Failed to start BZFlag.", "Failed to start BZFlag");
				return false;
			}

			return true;
		}

		private bool bzflagStop()
		{
			if (procBZFlag.StartInfo.FileName.Length != 0 && !procBZFlag.HasExited)
			{
				procBZFlag.Kill();
			}

			return true;
		}

		private void procBZFS_Exited(object sender, EventArgs e)
		{
			btnRun.Enabled = true;
			btnStop.Enabled = false;
		}
	}
}
