namespace StartBZFS
{
    partial class Paths
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.ClientPath = new System.Windows.Forms.TextBox();
            this.BrowseClient = new System.Windows.Forms.Button();
            this.AutoFindClient = new System.Windows.Forms.Button();
            this.AutoFindServer = new System.Windows.Forms.Button();
            this.BrowseServer = new System.Windows.Forms.Button();
            this.ServerPath = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.AutoFindWorld = new System.Windows.Forms.Button();
            this.BrowseWorld = new System.Windows.Forms.Button();
            this.WorldPath = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.OK = new System.Windows.Forms.Button();
            this.Cancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(95, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "BZFlag Client Path";
            // 
            // ClientPath
            // 
            this.ClientPath.Location = new System.Drawing.Point(15, 25);
            this.ClientPath.Name = "ClientPath";
            this.ClientPath.Size = new System.Drawing.Size(390, 20);
            this.ClientPath.TabIndex = 1;
            // 
            // BrowseClient
            // 
            this.BrowseClient.Location = new System.Drawing.Point(330, 51);
            this.BrowseClient.Name = "BrowseClient";
            this.BrowseClient.Size = new System.Drawing.Size(75, 23);
            this.BrowseClient.TabIndex = 2;
            this.BrowseClient.Text = "Browse";
            this.BrowseClient.UseVisualStyleBackColor = true;
            this.BrowseClient.Click += new System.EventHandler(this.BrowseClient_Click);
            // 
            // AutoFindClient
            // 
            this.AutoFindClient.Location = new System.Drawing.Point(15, 51);
            this.AutoFindClient.Name = "AutoFindClient";
            this.AutoFindClient.Size = new System.Drawing.Size(75, 23);
            this.AutoFindClient.TabIndex = 3;
            this.AutoFindClient.Text = "Auto Find";
            this.AutoFindClient.UseVisualStyleBackColor = true;
            this.AutoFindClient.Click += new System.EventHandler(this.AutoFindClient_Click);
            // 
            // AutoFindServer
            // 
            this.AutoFindServer.Location = new System.Drawing.Point(15, 128);
            this.AutoFindServer.Name = "AutoFindServer";
            this.AutoFindServer.Size = new System.Drawing.Size(75, 23);
            this.AutoFindServer.TabIndex = 7;
            this.AutoFindServer.Text = "Auto Find";
            this.AutoFindServer.UseVisualStyleBackColor = true;
            this.AutoFindServer.Click += new System.EventHandler(this.AutoFindServer_Click);
            // 
            // BrowseServer
            // 
            this.BrowseServer.Location = new System.Drawing.Point(330, 128);
            this.BrowseServer.Name = "BrowseServer";
            this.BrowseServer.Size = new System.Drawing.Size(75, 23);
            this.BrowseServer.TabIndex = 6;
            this.BrowseServer.Text = "Browse";
            this.BrowseServer.UseVisualStyleBackColor = true;
            this.BrowseServer.Click += new System.EventHandler(this.BrowseServer_Click);
            // 
            // ServerPath
            // 
            this.ServerPath.Location = new System.Drawing.Point(15, 102);
            this.ServerPath.Name = "ServerPath";
            this.ServerPath.Size = new System.Drawing.Size(390, 20);
            this.ServerPath.TabIndex = 5;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 86);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(100, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "BZFlag Server Path";
            // 
            // AutoFindWorld
            // 
            this.AutoFindWorld.Location = new System.Drawing.Point(15, 207);
            this.AutoFindWorld.Name = "AutoFindWorld";
            this.AutoFindWorld.Size = new System.Drawing.Size(75, 23);
            this.AutoFindWorld.TabIndex = 11;
            this.AutoFindWorld.Text = "Auto Find";
            this.AutoFindWorld.UseVisualStyleBackColor = true;
            this.AutoFindWorld.Click += new System.EventHandler(this.AutoFindWorld_Click);
            // 
            // BrowseWorld
            // 
            this.BrowseWorld.Location = new System.Drawing.Point(330, 207);
            this.BrowseWorld.Name = "BrowseWorld";
            this.BrowseWorld.Size = new System.Drawing.Size(75, 23);
            this.BrowseWorld.TabIndex = 10;
            this.BrowseWorld.Text = "Browse";
            this.BrowseWorld.UseVisualStyleBackColor = true;
            this.BrowseWorld.Click += new System.EventHandler(this.BrowseWorld_Click);
            // 
            // WorldPath
            // 
            this.WorldPath.Location = new System.Drawing.Point(15, 181);
            this.WorldPath.Name = "WorldPath";
            this.WorldPath.Size = new System.Drawing.Size(390, 20);
            this.WorldPath.TabIndex = 9;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 165);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(60, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "World Path";
            // 
            // OK
            // 
            this.OK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.OK.Location = new System.Drawing.Point(330, 241);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(75, 23);
            this.OK.TabIndex = 12;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            this.OK.Click += new System.EventHandler(this.OK_Click);
            // 
            // Cancel
            // 
            this.Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Cancel.Location = new System.Drawing.Point(249, 241);
            this.Cancel.Name = "Cancel";
            this.Cancel.Size = new System.Drawing.Size(75, 23);
            this.Cancel.TabIndex = 13;
            this.Cancel.Text = "Cancel";
            this.Cancel.UseVisualStyleBackColor = true;
            // 
            // Paths
            // 
            this.AcceptButton = this.OK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.Cancel;
            this.ClientSize = new System.Drawing.Size(417, 276);
            this.Controls.Add(this.Cancel);
            this.Controls.Add(this.OK);
            this.Controls.Add(this.AutoFindWorld);
            this.Controls.Add(this.BrowseWorld);
            this.Controls.Add(this.WorldPath);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.AutoFindServer);
            this.Controls.Add(this.BrowseServer);
            this.Controls.Add(this.ServerPath);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.AutoFindClient);
            this.Controls.Add(this.BrowseClient);
            this.Controls.Add(this.ClientPath);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "Paths";
            this.Text = "Paths";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox ClientPath;
        private System.Windows.Forms.Button BrowseClient;
        private System.Windows.Forms.Button AutoFindClient;
        private System.Windows.Forms.Button AutoFindServer;
        private System.Windows.Forms.Button BrowseServer;
        private System.Windows.Forms.TextBox ServerPath;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button AutoFindWorld;
        private System.Windows.Forms.Button BrowseWorld;
        private System.Windows.Forms.TextBox WorldPath;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button OK;
        private System.Windows.Forms.Button Cancel;
    }
}