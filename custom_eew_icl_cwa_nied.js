//混合ICL，台湾，日本的预警数据源（仅限GET方式）
//【重要】请将该文件命名为custom.js并放在与设置文件相同的文件夹内

//===========预警获取函数==============
var urls=[
    "https://mobile-new.chinaeew.cn/v1/earlywarnings?start_at=&updates=",//成都高新
    "https://api.wolfx.jp/cwa_eew.json",//台湾气象署
    "https://api.wolfx.jp/nied_eew.json"//防灾科研
];
var url_index=0;

//指定一个URL，若不想使用脚本的预警或地震历史功能请将相应的URL指定为空字符串，即""
function eew_url(){return urls[url_index];}

//指定请求方式，"get"或"post"等，还可指定为"websocket"使用WebSocket连接（此时URL应为ws或wss协议）
function eew_method(){return "get";}

//指定HTTP请求头，以JSON形式返回，使用WebSocket时此函数不会被调用
function eew_header(){return {/*"Accept":"application/json"*/};}

//请求方式为POST时提交的数据，或WebSocket连接成功后要发送的数据，字符串形式，空字符串表示不发送
function eew_postdata(){return "";}

//成功返回数据时请将响应内容转换为指定的JSON形式
//格式如下：
//  {data:[{eventId:数值型事件ID,
//          updates:数值型第几报,
//          latitude:数值型震中纬度,
//          longitude:数值型震中经度,
//          depth:数值型震源深度（公里）,
//          epicenter:"字符串型震源地名称",
//          startAt:数值型发震时间戳（毫秒）,
//          magnitude:数值型震级},
//         {...},{...},{...},...
//        ]}
var last_eews=[
    [],//成都高新
    [],//台湾气象署
    []//防灾科研
];
function eew_onsuccess(str_response){
    //有ID属性的是CWA，有eew属性的是NIED，有data属性的是ICL
    url_index=(url_index+1)%urls.length;
    var original=JSON.parse(str_response);
    //判断类型
    if(original.data!==undefined){
        //转换来自ICL的格式
        last_eews[0]=original.data;
    }else if(original.ID!==undefined){
        //转换来自CWA的格式
        last_eews[1]=[{
            eventId:original.ID,
            updates:original.ReportNum,
            latitude:original.Latitude,
            longitude:original.Longitude,
            depth:original.Depth,
            epicenter:original.HypoCenter,
            startAt:fmt_to_msts(original.OriginTime+" UTC+8"),//注意时区问题
            magnitude:original.Magunitude
        }];
    }else if(original.eew!==undefined){
        //转换来自NIED的格式
        last_eews[2]=[{
            eventId:fmt_to_msts(original.report_time+" UTC+9"),
            updates:parseInt(original.report_num),
            latitude:parseFloat(original.latitude),
            longitude:parseFloat(original.longitude),
            depth:parseFloat(original.depth),
            epicenter:original.region_name,
            startAt:fmt_to_msts(original.origin_time.substr(0,4)+"-"+
            original.origin_time.substr(4,2)+"-"+
            original.origin_time.substr(6,2)+" "+
            original.origin_time.substr(8,2)+":"+
            original.origin_time.substr(10,2)+":"+
            original.origin_time.substr(12,2)+" UTC+9"),//注意时区问题
            magnitude:parseFloat(original.magunitude)
        }];
    }
    //合并
    var combined={data:[]};
    for(var i=0;i<urls.length;i++){
        combined.data=combined.data.concat(last_eews[i]);
    }
    //按发震时间降序排序
    combined.data.sort((a,b)=>{return b.startAt-a.startAt;});
    return combined;
}

//失败时的调用，参数为一个数值型的错误码
function eew_onfail(num_errorcode){url_index=(url_index+1)%urls.length;}

//根据URL判断该URL返回的是否为EEW数据，使用WebSocket时此函数不会被调用
function is_eew_data(url){return url===urls[url_index];}


//=========辅助函数=============

//将毫秒数时间戳转为YYYY-MM-DD HH:MM:SS
function msts_to_fmt(msts){
    var isodt=new Date(msts).toISOString();
    return isodt.substr(0,10)+" "+isodt.substr(11,8);
}

//将YYYY-MM-DD HH:MM:SS转为毫秒数时间戳
function fmt_to_msts(fmt){
    return new Date(fmt).getTime();
}
