namespace StartBZFS
{
    partial class ServerLogWindow
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
            this.ServerLog = new System.Windows.Forms.TextBox();
            this.Stop = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // ServerLog
            // 
            this.ServerLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.ServerLog.Location = new System.Drawing.Point(0, 1);
            this.ServerLog.Multiline = true;
            this.ServerLog.Name = "ServerLog";
            this.ServerLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.ServerLog.Size = new System.Drawing.Size(272, 245);
            this.ServerLog.TabIndex = 0;
            // 
            // Stop
            // 
            this.Stop.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.Stop.Location = new System.Drawing.Point(190, 252);
            this.Stop.Name = "Stop";
            this.Stop.Size = new System.Drawing.Size(75, 23);
            this.Stop.TabIndex = 1;
            this.Stop.Text = "Stop";
            this.Stop.UseVisualStyleBackColor = true;
            this.Stop.Click += new System.EventHandler(this.Stop_Click);
            // 
            // ServerLogWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(277, 282);
            this.Controls.Add(this.Stop);
            this.Controls.Add(this.ServerLog);
            this.Name = "ServerLogWindow";
            this.Text = "ServerLogWindow";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.ServerLogWindow_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox ServerLog;
        private System.Windows.Forms.Button Stop;
    }
}