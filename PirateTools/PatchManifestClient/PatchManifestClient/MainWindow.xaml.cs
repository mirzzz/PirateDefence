using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Text.Json;
using System.IO;


namespace PatchManifestClient
{
    public partial class MainWindow : Window
    {
        public string ConfigFileName = "PatchManifestClient.json";

        // 버전 파일 이름
        public string ServerVersionName = "ServerVersion.txt";

        // 패치 파일 이름
        public string ServerManifestName = "ServerManifest-{0}.json";

        // Platform Items
        public PlatformList PlatformItems = new PlatformList();

        // Log Items
        public PakList      PakItems = new PakList();

        // Log Items
        public LogList      LogItems = new LogList();

        public MainWindow()
        {
            InitializeComponent();

            // 핸들러 추가.            
            this.Activated   += new EventHandler(OnApplicationActivated);
            this.Deactivated += new EventHandler(OnApplicationDeactivated);
            this.Closing    += new System.ComponentModel.CancelEventHandler(OnApplicationClosing);
            this.Closed     += new EventHandler(OnApplicationClosed);
                        
            PlatformCombo.ItemsSource = PlatformItems;
            PakListView.ItemsSource = PakItems;
            LogListBox.ItemsSource = LogItems;

            LoadConfigFile();
        }


        private void OnApplicationActivated(object? sender, EventArgs e) 
        {
            // 활성화
        }
        private void OnApplicationDeactivated(object? sender, EventArgs e)
        {
            // 비활성화
        }

        private void OnApplicationClosing(object? sender, System.ComponentModel.CancelEventArgs e)
        {
            MessageBoxResult closingMessegeBoxResult = MessageBox.Show("정말로 종료 할까요?", "종료 확인", MessageBoxButton.OKCancel, MessageBoxImage.Error);
            if (closingMessegeBoxResult != MessageBoxResult.OK)
            {
                e.Cancel = true;
            }
        }

        public void LoadConfigFile()
        {
            if(File.Exists(ConfigFileName)) 
            {
                string jsonString = File.ReadAllText(ConfigFileName);
                ConfigJsonInfo configInfo = JsonSerializer.Deserialize<ConfigJsonInfo>(jsonString)!;

                // config 설정
                IsSaveConfig.IsChecked      = configInfo.bConfigSave;
                PlatformCombo.SelectedIndex = configInfo.PlatformIdx;
                EmbeddedPathTBox.Text       = configInfo.EmbeddedPath;
                BuildPathTBox.Text          = configInfo.BuildPath;
                PatchVersionPathTBox.Text   = configInfo.FileVersionPath;
                PatchExportPathTBox.Text    = configInfo.FileExportPath;
            }
        }

        public bool SaveConfigFile()
        {
            var configInfo = new ConfigJsonInfo
            {
                bConfigSave   = IsSaveConfig.IsChecked.HasValue,
                PlatformIdx   = PlatformCombo.SelectedIndex,
                EmbeddedPath  = EmbeddedPathTBox.Text,
                BuildPath     = BuildPathTBox.Text,
                FileVersionPath = PatchVersionPathTBox.Text,
                FileExportPath  = PatchExportPathTBox.Text
            };

            // 파일 쓰기.
            JsonSerializerOptions jso =  new JsonSerializerOptions();
            jso.Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping;

            string jsonString = JsonSerializer.Serialize(configInfo, jso);
            File.WriteAllText(ConfigFileName, jsonString);
            Console.WriteLine(File.ReadAllText(ConfigFileName));

            return true;
        }

        private void OnApplicationClosed(object? sender, EventArgs e)
        {
            SaveConfigFile();
        }

        // 경로 설정
        private void DirectorySetting( TextBox textBox)
        {
            var dialog = new Microsoft.Win32.SaveFileDialog();
            dialog.InitialDirectory = textBox.Text;
            dialog.Title = "Select a Directory";
            dialog.Filter = "Directory|*.this.directory"; // Prevents displaying files
            dialog.FileName = "select";                   // Filename will then be "select.this.directory"
            if (dialog.ShowDialog() == true)
            {
                string path = dialog.FileName;
                // Remove fake filename from resulting path
                path = path.Replace("\\select.this.directory", "");
                path = path.Replace(".this.directory", "");
                
                // 경로 없을경우 생성
                if (!System.IO.Directory.Exists(path))
                {
                    System.IO.Directory.CreateDirectory(path);
                }                
                textBox.Text = path;
            }
        }

        private void EmbeddedPathTBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
        // 기존 프로젝트와 함께 배포되는 Pak 폴더
        private void EmbeddedFolderBtn_Click(object sender, RoutedEventArgs e)
        {
            DirectorySetting(EmbeddedPathTBox);
        }

        private void EmbeddedExplorerBtn_Click(object sender, RoutedEventArgs e)
        {
            string argument = "/select, \"" + EmbeddedPathTBox.Text + "\"";
            System.Diagnostics.Process.Start("Explorer.exe", argument);
        }


        private void BuildPathTBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            string BuildVersion = BuildPathTBox.Text;

            string[] folders = BuildVersion.Split('\\');

            string? platform = ((PlatformData)PlatformCombo.SelectedItem).name;
            for (int i = 0; i < folders.Length; ++i)
            {
                // 문자열 비교
                if (folders[i].Equals(platform, StringComparison.OrdinalIgnoreCase))
                {
                    PatchVersionTBox.Text = folders[i - 1];
                    break;
                }
            }
        }
                
        // 새롭게 빌드의 Pak 폴더
        private void BuildFolderBtn_Click(object sender, RoutedEventArgs e)
        {
            DirectorySetting(BuildPathTBox);

           string BuildVersion = BuildPathTBox.Text;
            
            string[] folders = BuildVersion.Split('\\');

            string? platform = ((PlatformData)PlatformCombo.SelectedItem).name;
            for (int i=0; i<folders.Length; ++i )
            {
                // 문자열 비교
                if(folders[i].Equals(platform, StringComparison.OrdinalIgnoreCase))
                {
                    PatchVersionTBox.Text = folders[i-1];
                    break;
                }
            }
        }

        private void BuildExplorerBtn_Click(object sender, RoutedEventArgs e)
        {
            string argument = "/select, \"" + BuildPathTBox.Text + "\"";
            System.Diagnostics.Process.Start("Explorer.exe", argument);
        }

        private void PatchVersionFolderBtn_Click(object sender, RoutedEventArgs e)
        {
            DirectorySetting(PatchVersionPathTBox);
        }
        private void PatchVersionExplorerBtn_Click(object sender, RoutedEventArgs e)
        {
            string argument = "/select, \"" + PatchVersionPathTBox.Text + "\"";
            System.Diagnostics.Process.Start("Explorer.exe", argument);
        }

        // 생성할 위치
        private void PatchExportFolderBtn_Click(object sender, RoutedEventArgs e)
        {
            DirectorySetting(PatchExportPathTBox);
        }

        private void PatchExportExplorerBtn_Click(object sender, RoutedEventArgs e)
        {
            string argument = "/select, \"" + PatchExportPathTBox.Text + "\"";
            System.Diagnostics.Process.Start("Explorer.exe", argument);
        }

        private void PakClearBtn_Click(object sender, RoutedEventArgs e)
        {
            PakItems.Clear();
            
        }
        private void LogClearBtn_Click(object sender, RoutedEventArgs e)
        {
            LogItems.Clear();
        }

        // 생성
        private void GenerateBtn_Click(object sender, RoutedEventArgs e)
        {
            LogItems.Insert(LogItems.Count, new LogData() { Message = string.Format("Generate Pak Version {0}", PatchVersionTBox.Text) });

            GenServerVersion();
            GenServerManifest();
        }

        private void GenServerVersion()
        {
            // ServerVersion.txt 파일 생성
            string outputPath    = string.Format("{0}\\{1}", PatchVersionPathTBox.Text, ServerVersionName);
            string versionString = PatchVersionTBox.Text;

            // 기본 파일 백업
            if (File.Exists(outputPath) == true)
            {
                string renamePath = string.Format("{0}\\ServerVersion-{1}.txt", PatchVersionPathTBox.Text, DateTime.Now.ToString("yyyyMMdd_HHmmss"));
                System.IO.File.Move(outputPath, renamePath);
            }
            
            System.IO.File.WriteAllText(outputPath, versionString, Encoding.Default);
        }

        //
        private void GenServerManifest()
        {
            PakItems.Clear();

            // EmbeddedPath 파일 가져오기
            DirectoryInfo dirEmbeddedInfo = new DirectoryInfo(EmbeddedPathTBox.Text);
            FileInfo[] Embeddedinfos = dirEmbeddedInfo.GetFiles("*.*", SearchOption.AllDirectories);

            Dictionary<string, FileInfo> EmbeddedFile = new Dictionary<string, FileInfo>();
            for (int ef = 0; ef < Embeddedinfos.Length; ++ef)
            {
                //PakItems.Insert(PakItems.Count, new PakData() { Category = "Embedded", Filename = Embeddedinfos[ef].Name, CreateDate = Embeddedinfos[ef].CreationTime, ModifyDate = Embeddedinfos[ef].LastAccessTime, });
                EmbeddedFile.Add(Embeddedinfos[ef].Name, Embeddedinfos[ef]);
            }

            // BulidPath 파일 가져오기
            DirectoryInfo dirBuildInfo = new DirectoryInfo(BuildPathTBox.Text);
            FileInfo[] Buildinfos = dirBuildInfo.GetFiles("*.*", SearchOption.AllDirectories);

            Dictionary<string, FileInfo> NewFile = new Dictionary<string, FileInfo>();
            for (int bf = 0; bf < Buildinfos.Length; ++bf)
            {
                //PakItems.Insert(PakItems.Count, new PakData() { Category = "Build", Filename = Buildinfos[bf].Name, CreateDate = Buildinfos[bf].CreationTime, ModifyDate = Buildinfos[bf].LastAccessTime, });
                if (EmbeddedFile.ContainsKey(Buildinfos[bf].Name) == false)
                {
                    NewFile.Add(Buildinfos[bf].Name, Buildinfos[bf]);
                }
            }

            // 업데이트 파일
            LogItems.Insert(LogItems.Count, new LogData() { Message = string.Format("Update File Count : {0}", NewFile.Count) });
            
            // json 생성
            ServerManifestJsonInfo serverManifest = new ServerManifestJsonInfo();

            {
                serverManifest.FileVersion = "1";
                serverManifest.ContentPlatform = ((PlatformData)PlatformCombo.SelectedItem).name;
                serverManifest.ContentVersion = PatchVersionTBox.Text;
                serverManifest.PakCount = NewFile.Count;
                serverManifest.PakFiles = new List<PakFileInfo>();

                string prevWord = "pakchunk";
                string platformWord = string.Format("-{0}", serverManifest.ContentPlatform);

                foreach (FileInfo file in NewFile.Values)
                {
                    PakFileInfo pakFile = new PakFileInfo();

                    pakFile.PakName = file.Name;
                    pakFile.PakBytes = file.Length;
                    pakFile.PakVersion = PatchVersionTBox.Text;

                    int prevPos = prevWord.Length;
                    int lastPos = file.Name.IndexOf(platformWord);

                    string strChunkID = file.Name.Substring(prevPos, lastPos - prevPos);

                    pakFile.PakChunkId = Int32.Parse(strChunkID);
                    pakFile.PakRelativeUrl = string.Format("/{0}/{1}", pakFile.PakVersion, pakFile.PakName);
                    pakFile.PakType = "Build";

                    serverManifest.PakFiles.Add(pakFile);

                    PakItems.Insert(PakItems.Count, new PakData()
                    {
                        Category    = pakFile.PakType, 
                        Filename    = pakFile.PakName, 
                        ChunkID     = pakFile.PakChunkId, 
                        Filesize    = pakFile.PakBytes,
                        CreateDate  = file.CreationTime,
                        ModifyDate  = file.LastAccessTime
                    });
                }
                serverManifest.Description = "MakePatchTools: 정렬[Ctrl+K,F]";
            }

            // utf8
            JsonSerializerOptions jso = new JsonSerializerOptions();
            jso.Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping;

            // ServerManifest-%s.json 파일 생성
            string scriptFileName = string.Format(ServerManifestName, PatchVersionTBox.Text);

            string jsonString = JsonSerializer.Serialize(serverManifest, jso);
            string outputPath = string.Format("{0}\\{1}", PatchExportPathTBox.Text, scriptFileName);

            // 이미 파일이 있는경우, 덮어쓸지 다른 이름으로 저장할지.
            if(File.Exists(outputPath) == true)
            {
                MessageBoxResult closingMessegeBoxResult = 
                    MessageBox.Show(
                        string.Format( "{0} 파일이 이미 있습니다.\r\n파일을 덮어 쓰시겠습니까?", scriptFileName),
                        "파일 저장 확인", MessageBoxButton.OKCancel, MessageBoxImage.Error);

                if (closingMessegeBoxResult != MessageBoxResult.OK)
                {
                    Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
                    dlg.InitialDirectory = PatchExportPathTBox.Text;
                    dlg.FileName = scriptFileName;  
                    dlg.DefaultExt = ".json";   
                    dlg.Filter = "json file (.json)|*.json";

                    Nullable<bool> result = dlg.ShowDialog();
                    if (result == true)
                    {
                        outputPath = dlg.FileName;
                    }
                    else
                    {
                        return;  // 저장 안함.
                    }
                }
            }

            // 파일 복사.
            string srcPath = BuildPathTBox.Text;
            string dstPath = string.Format("{0}\\{1}", PatchExportPathTBox.Text, PatchVersionTBox.Text);
            LogItems.Insert(LogItems.Count, new LogData() { Message = string.Format("Copy PakFile : {0}", serverManifest.PakFiles.Count) });
            Task.Run(() => CopyPakFiles(serverManifest.PakFiles, srcPath, dstPath));

            // Script 파일 생성
            LogItems.Insert(LogItems.Count, new LogData() { Message = string.Format("Generate JsonFile : {0}", outputPath) });
            File.WriteAllText(outputPath, jsonString);
        }

        private void CopyPakFiles(IList<PakFileInfo> pakfiles, string srcPath, string destPath)
        {
            // 폴더 없을경우 생성
            DirectoryInfo destDir = new DirectoryInfo(destPath);
            if (destDir.Exists == false)
                destDir.Create();

            List<PakFileInfo> copyFiles = pakfiles.ToList();

            // 모든 파일을 복사하고 같은 이름의 모든 파일 바꾸기            
            GenerateProgress.Dispatcher.Invoke(() =>
            {
                GenerateProgress.Visibility = Visibility.Visible;
                GenerateProgress.Maximum = copyFiles.Count;
                GenerateProgress.Value = 0;
            });
            for ( int i=0; i< copyFiles.Count; ++i )
            {
                PakFileInfo pakfile = copyFiles[i];

                string srcFilename = string.Format("{0}\\{1}", srcPath, pakfile.PakName);
                string dstFilename = string.Format("{0}\\{1}", destPath, pakfile.PakName);

                if( File.Exists(srcFilename) )
                    File.Copy(srcFilename, dstFilename, true);

                GenerateProgress.Dispatcher.Invoke(() => GenerateProgress.Value += i + 1);
            }
            GenerateProgress.Dispatcher.Invoke(() => GenerateProgress.Visibility = Visibility.Collapsed);
        }
    }
}
