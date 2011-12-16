using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;
using System.IO;

namespace SerialDataDownload
{
    public partial class SerialConnection : Form
    {
        private String mPortName = "COM1";
        private SerialPort mPort = null;
        private OpenFileDialog openFileDialog = null;

        public SerialConnection()
        {
            InitializeComponent();
        }

        public SerialConnection(String port) : this()
        {
            mPortName = port;
            openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "csv files (*.csv)|*.csv|All files (*.*)|*.*";
            openFileDialog.FilterIndex = 1;
            openFileDialog.RestoreDirectory = true;
        }

        private void Connect(object unused)
        {
            mPort = new SerialPort(mPortName, 115200, Parity.None, 8, StopBits.One);
            mPort.Handshake = Handshake.RequestToSend;
            mPort.RtsEnable = true;
            mPort.Encoding = Encoding.ASCII;
            while (true)
            {
                this.BeginInvoke(new Action<String>(AddMessage), "Connecting...");
                try
                {
                    mPort.Open();                    
                    break;
                }
                catch (Exception ex)
                {
                    if (mPort.IsOpen)
                    {
                        mPort.Close();
                    }
                    this.BeginInvoke(new Action<String>(AddMessage), "Error: " + ex.Message);
                    Thread.Sleep(1000);
                }
            }
            this.BeginInvoke(new Action(() => 
            { 
                AddMessage("Connected");
                DownloadButton.Enabled = true; 
            }));
        }

        private void AddMessage(String message)
        {
            TextOutput.Text += (message + "\r\n");
        }

        private void SerialConnection_FormClosed(object sender, FormClosedEventArgs e)
        {
            if ((mPort != null) && (mPort.IsOpen))
            {
                mPort.Close();
            }
            Application.Exit();
        }

        private void SerialConnection_Load(object sender, EventArgs e)
        {
            ThreadPool.QueueUserWorkItem(new WaitCallback(Connect));
        }

        private void DownloadButton_Click(object sender, EventArgs e)
        {
            ThreadPool.QueueUserWorkItem(new WaitCallback(Download));
        }

        private void Download(object unused)
        {
            try
            {
                this.BeginInvoke(new Action<String>(AddMessage), "Send: download");
                write(mPort, "download");
                Thread.Sleep(1000);
                while (mPort.BytesToRead > 0)
                {
                    String data = mPort.ReadExisting();
                    this.BeginInvoke(new Action<String>(AddMessage), data);
                }                
            }
            catch (Exception ex)
            {
                this.BeginInvoke(new Action<String>(AddMessage), "Error: " + ex.Message);
            }
        }

        private void write(SerialPort mPort, string str)
        {
            foreach (char c in str)
            {
                mPort.Write(new char[] {c}, 0, 1);
                Thread.Sleep(10);
            }
        }

        private void ClearButton_Click(object sender, EventArgs e)
        {
            TextOutput.Clear();
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    File.WriteAllText(openFileDialog.FileName,
                                      TextOutput.Text);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: Could not write file to disk. Original error: " + ex.Message);
                }
            }
        }
    }
}
