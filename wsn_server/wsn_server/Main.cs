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
using System.Globalization;

namespace wsn_server
{
    public partial class Main : Form
    {
        private String mPortName = null;
        private SerialPort mPort = null;
        private SaveFileDialog saveFileDialog = null;
        private OpenFileDialog openFileDialog = null;
        private bool connected = false;

        public Main()
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

        public Main(String port) : this()
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
            mPort = new SerialPort(mPortName, 115200, Parity.None, 8, StopBits.One);
            mPort.Handshake = Handshake.RequestToSend;
            mPort.Encoding = Encoding.ASCII;
            while (true)
            {
                this.BeginInvoke(new Action<String>(AddMessageLine), "Connecting...");
                try
                {
                    mPort.Open();
                    connected = true;
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

            BeginDownload();
        }

        private void BeginDownload()
        {
            TimeSpan t = (DateTime.UtcNow - new DateTime(1970, 1, 1));
            long timestamp = (long)t.TotalSeconds;

            SendCommand("set_time_" + timestamp, ListFiles);
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
            this.CommandTextBox.Focus();
        }

        private void DownloadButton_Click(object sender, EventArgs e)
        {
            ThreadPool.QueueUserWorkItem(new WaitCallback((object x) => { BeginDownload(); }));
        }

        private void ListFiles(string setTimeResp)
        {
            dataList = new PointPairList();
            SendCommand("ls", DownloadFiles);
        }

        private void DownloadFiles(string fileList)
        {
            string[] lines = fileList.Split('\n');
            foreach (string line in lines)
            {
                if (!line.StartsWith("=="))
                {
                    string[] parts = line.Split(' ');
                    string filename = parts[0];
                    filename = filename.Trim();

                    if (filename.Length > 0)
                    {
                        SendCommand("cat " + filename, ProcessFile);
                    }
                }
            }
            this.BeginInvoke(new Action<String>(AddMessageLine), "Found " + dataList.Count + " data items!");
        }

        private void ProcessFile(string fileData)
        {
            ParseData(fileData);
        }

        delegate void Callback(string data);
        private void SendCommand(string command, Callback callback)
        {
            try
            {
                this.BeginInvoke(new Action<String>(AddMessageLine), "Send: " + command);
                mPort.Write(command);

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
                    callback(downloadedStr);
                }
            }
            catch (Exception ex)
            {
                this.BeginInvoke(new Action<String>(AddMessageLine), "Error: " + ex.Message);
            }
        }

        private static readonly DateTime Epoch = new DateTime(1970, 1, 1, 0, 0, 0, 
                                                      DateTimeKind.Utc);

        public static DateTime UnixTimeToDateTime(string text)
        {
            double seconds = double.Parse(text, CultureInfo.InvariantCulture);
            return Epoch.AddSeconds(seconds);
        }

        PointPairList dataList = new PointPairList();
        private void ParseData(string downloadedStr)
        {            
            string[] lines = downloadedStr.Split(new char[] {'\n'}, 
                                                 StringSplitOptions.RemoveEmptyEntries);
            foreach (string line in lines)
            {
                Match match = Regex.Match(line,
                                    @"[\d]+,[\d]+\.[\d]+",
                                    RegexOptions.IgnorePatternWhitespace);
                if (match.Success)
                {
                    string matchVal = match.Value;
                    string[] matchParts = matchVal.Split(new char[] { ',' });

                    string timeStr = matchParts[0];
                    DateTime dataDateTime = UnixTimeToDateTime(timeStr);                    

                    string tempStr = matchParts[1];
                    double tempValue = Double.Parse(tempStr);

                    double x, y;
                    XDate graphDataDateTime = new XDate(dataDateTime);
                    x = (double)graphDataDateTime;
                    y = tempValue;
                    dataList.Add(x, y);
                }
            }
            
            this.BeginInvoke(new Action(() =>
            {
                Graph.GraphPane.CurveList.Clear();
                Graph.GraphPane.AddCurve("Temp", dataList, Color.Black, SymbolType.None);
                Graph.GraphPane.Title.Text = "Temp";
                Graph.AxisChange();
                Graph.Refresh();
            }));
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
                        string dataToWrite = DateTime.Now.ToString();
                        dataToWrite += Environment.NewLine;
                        dataToWrite += TextOutput.Text;
                        File.WriteAllText(saveFileDialog.FileName,
                                          dataToWrite);
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
                        string[] fileDataParts = fileData.Split(new String[] {Environment.NewLine}, 2, StringSplitOptions.RemoveEmptyEntries);
                        if (fileDataParts.Length < 2)
                        {
                            throw new Exception("Invalid file format");
                        }
                        string fileDataDateStr = fileDataParts[0];
                        DateTime fileDataDate = DateTime.Parse(fileDataDateStr);
                        string fileDataPart = fileDataParts[1];

                        /*this.BeginInvoke(new Action(TextOutput.Clear));
                        this.BeginInvoke(new Action<string>(AddMessage), fileDataPart);
                        ParseData(fileDataPart, fileDataDate);*/
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
                    }
                }));
            }
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            DoSend();
        }

        private void DoSend()
        {
            if (connected)
            {
                string commandstr = CommandTextBox.Text;
                CommandTextBox.Text = "";
                ThreadPool.QueueUserWorkItem(new WaitCallback(
                    (object x) =>
                    {
                        SendCommand(commandstr, NoOp);
                    }));
            }
        }

        private void NoOp(string value) { }

        private void CommandTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                DoSend();
        }
    }
}
