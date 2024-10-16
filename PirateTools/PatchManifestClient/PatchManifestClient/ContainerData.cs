using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PatchManifestClient
{
    // ComboBox - Platform 
    public class PlatformData
    {
        public string? name { get; set; }
        public string? value { get; set; }
    }

    public class PlatformList: List<PlatformData>
    {
        public PlatformList()
        {
            Add(new PlatformData() { name = "Windows", value = "1" });
            Add(new PlatformData() { name = "Android", value = "2" });
        }
    }


    // ListView - Pak
    public class PakData
    {
        public string? Category { get; set; }
        public string? Filename { get; set; }
        public Int64  Filesize { get; set; }
        public Int32  ChunkID { get; set; }
        public DateTime CreateDate { get; set; }
        public DateTime ModifyDate { get; set; }

        public PakData()
        {
            Category = "";
            ChunkID  = 0;
            Filesize = 0;
        }
    }
    public class PakList : ObservableCollection<PakData>
    {
        public PakList()
        {   
        }
    }


    // ListBox - Log 
    public class LogData
    {
        public string? Message { get; set; }

        public string? Tail { get; set; }
    }

    public class LogList : ObservableCollection<LogData>
    {
        public LogList()
        {
            Add(new LogData() { Tail = "", Message = "Log Message..." });
        }
    }

}

