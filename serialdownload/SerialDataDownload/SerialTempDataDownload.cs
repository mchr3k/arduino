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
using System.Text.RegularExpressions;
using System.Diagnostics;
using ZedGraph;

namespace SerialDataDownload
{
    public partial class SerialTempDataDownload : Form
    {
        private String mPortName = null;
        private SerialPort mPort = null;
        private SaveFileDialog saveFileDialog = null;
        private OpenFileDialog openFileDialog = null;

        public SerialTempDataDownload()
        {
            InitializeComponent();

            saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "csv files (*.csv)|*.csv|All files (*.*)|*.*";
            saveFileDialog.FilterIndex = 1;
            saveFileDialog.RestoreDirectory = true;

            openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "csv files (*.csv)|*.csv|All files (*.*)|*.*";
            openFileDialog.FilterIndex = 1;
            openFileDialog.RestoreDirectory = true;

            PrepareGraph();
        }

        public SerialTempDataDownload(String port) : this()
        {
            this.Text += " - " + port;
            mPortName = port;
        }

        private void PrepareGraph()
        {
            Graph.IsAntiAlias = true;
            Graph.IsShowPointValues = true;
            Graph.GraphPane.Title.Text = "Waiting for data...";
            Graph.GraphPane.XAxis.Type = AxisType.Date;
            Graph.GraphPane.XAxis.Title.Text = "Time";
            Graph.GraphPane.YAxis.Title.Text = "Temp";
        }

        private void Connect(object unused)
        {
            mPort = new SerialPort(mPortName, 9600, Parity.None, 8, StopBits.One);
            mPort.Handshake = Handshake.RequestToSend;
            mPort.Encoding = Encoding.ASCII;
            while (true)
            {
                this.BeginInvoke(new Action<String>(AddMessageLine), "Connecting...");
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
                    this.BeginInvoke(new Action<String>(AddMessageLine), "Error: " + ex.Message);
                    Thread.Sleep(1000);
                }
            }
            this.BeginInvoke(new Action(() => 
            { 
                AddMessageLine("Connected");
                DownloadButton.Enabled = true; 
            }));
            Download(null);
        }

        private void AddMessageLine(String message)
        {
            AddMessage("> " + message + "\r\n");
        }

        private void AddMessage(String message)
        {
            TextOutput.AppendText(message);
        }

        private void SerialConnection_Load(object sender, EventArgs e)
        {
            if (mPortName != null)
            {
                ThreadPool.QueueUserWorkItem(new WaitCallback(Connect));
            }
            else
            {
                DownloadButton.Enabled = false;
            }
        }

        private void DownloadButton_Click(object sender, EventArgs e)
        {
            ThreadPool.QueueUserWorkItem(new WaitCallback(Download));
        }

        private void Download(object unused)
        {
            try
            {
                this.BeginInvoke(new Action<String>(AddMessageLine), "Send: download");
                mPort.Write("download");

                Stopwatch sw = Stopwatch.StartNew();
                string downloadedStr = "";
                while (sw.ElapsedMilliseconds < 500)
                {
                    if (mPort.BytesToRead > 0)
                    {
                        String data = mPort.ReadExisting();
                        downloadedStr += data;
                        this.BeginInvoke(new Action<String>(AddMessage), data);
                        sw = Stopwatch.StartNew();
                    }
                }
                if (downloadedStr.Length > 0)
                {
                    parseData(downloadedStr);
                }
            }
            catch (Exception ex)
            {
                this.BeginInvoke(new Action<String>(AddMessageLine), "Error: " + ex.Message);
            }
        }

        private void parseData(string downloadedStr)
        {
            string[] parts = downloadedStr.Split(new String[] {"------"}, 
                                                 StringSplitOptions.None);
            foreach (string part in parts)
            {
                Match match = Regex.Match(part,
                                    @"[\d]+,[\d]+\.[\d]+",
                                    RegexOptions.Multiline | RegexOptions.IgnorePatternWhitespace);
                if (match.Success)
                {
                    List<string> dataStrs = new List<string>();
                    while (match.Success)
                    {
                        string matchVal = match.Value;
                        string[] matchParts = matchVal.Split(new char[] { ',' });
                        string matchStr = matchParts[1];
                        dataStrs.Add(matchStr);
                        match = match.NextMatch();
                    }

                    DateTime tempCaptureTime = DateTime.Now;
                    double x, y;
                    PointPairList dataList = new PointPairList();
                    int ii = 0;
                    foreach (string data in dataStrs)
                    {
                        double tempValue = Double.Parse(data);
                        TimeSpan span = new TimeSpan(0, // hours 
                                                     (dataStrs.Count - ii) * 7, // minutes
                                                     0); // seconds
                        DateTime dataDateTime = tempCaptureTime.Subtract(span);
                        XDate graphDataDateTime = new XDate(dataDateTime);
                        x = (double)graphDataDateTime;
                        y = tempValue;
                        dataList.Add(x, y);//, "(" + x + "," + y + ")");
                        ii++;
                    }
                    this.BeginInvoke(new Action<String>(AddMessageLine), "Found " + dataList.Count + " data items!");
                    this.BeginInvoke(new Action(() => 
                    {
                        Graph.GraphPane.CurveList.Clear();
                        Graph.GraphPane.AddCurve("Temp", dataList, Color.Black, SymbolType.None);
                        Graph.GraphPane.Title.Text = "Temp";
                        Graph.AxisChange();
                        Graph.Refresh();
                    }));
                }
            }
        }

        private void ClearButton_Click(object sender, EventArgs e)
        {
            TextOutput.Clear();
            Graph.GraphPane.CurveList.Clear();
            Graph.Refresh();
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                string selectedFile = saveFileDialog.FileName;
                ThreadPool.QueueUserWorkItem(new WaitCallback((object x) =>
                {
                    try
                    {
                        File.WriteAllText(saveFileDialog.FileName,
                                          TextOutput.Text);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Error: Could not write file to disk. Original error: " + ex.Message);
                    }
                }));                
            }
        }

        private void SerialConnection_FormClosed(object sender, FormClosedEventArgs e)
        {
            Application.Exit();
        }

        private void LoadButton_Click(object sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                string selectedFile = openFileDialog.FileName;
                ThreadPool.QueueUserWorkItem(new WaitCallback((object x) =>
                {
                    try
                    {
                        string fileData = File.ReadAllText(selectedFile);
                        this.BeginInvoke(new Action(TextOutput.Clear));
                        this.BeginInvoke(new Action<string>(AddMessage), fileData);
                        parseData(fileData);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
                    }
                }));
            }
        }
    }
}
