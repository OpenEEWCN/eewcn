//自定义数据源示例
//【重要】请将该文件命名为custom.js并放在与设置文件相同的文件夹内

//===========预警获取函数==============

//指定一个URL，若不想使用脚本的预警或地震历史功能请将相应的URL指定为空字符串，即""
function eew_url(){return "https://api.wolfx.jp/sc_eew.json";}

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
function eew_onsuccess(str_response){
    var original=JSON.parse(str_response);
    var converted={
        eventId:original.ID,
        updates:original.ReportNum,
        latitude:original.Latitude,
        longitude:original.Longitude,
        depth:original.Depth,
        epicenter:original.HypoCenter,
        startAt:fmt_to_msts(original.OriginTime+" UTC+8"),//注意时区问题
        magnitude:original.Magunitude
    };
    return {data:[converted]};
}

//失败时的调用，参数为一个数值型的错误码
function eew_onfail(num_errorcode){}

//根据URL判断该URL返回的是否为EEW数据，使用WebSocket时此函数不会被调用
function is_eew_data(url){return url==="https://api.wolfx.jp/sc_eew.json";}

//=========地震历史数据获取函数，格式同上=============

function history_url(){return "https://api.wolfx.jp/cenc_eqlist.json";}
function history_method(){return "get";}
function history_header(){return {/*"Accept":"application/json"*/};}
function history_postdata(){return "";}

//格式如下：
//  {shuju:[{id:"字符串型事件ID（在程序中会被转换为32位int型）",
//          O_TIME:"YYYY-MM-DD HH:MM:SS格式发震时间",
//          EPI_LAT:"字符串型震中纬度",
//          EPI_LON:"字符串型震中经度",
//          EPI_DEPTH:数值型震源深度（公里）,
//          AUTO_FLAG:"字符串型自动测定标记，例如A，M等"
//          EQ_TYPE:"M",
//          M:"字符串型震级"
//          LOCATION_C:"字符串型震源地名称"},
//         {...},{...},{...},...
//        ]}
function history_onsuccess(str_response){
    var original=JSON.parse(str_response);
    var shuju_array=[];
    for(var i=1;i<=50;i++){
        var item=original["No"+i];
        shuju_array.push({
            id:(fmt_to_msts(item.time+" UTC+8")/1000).toString(),
            O_TIME:item.time,
            EPI_LAT:item.latitude,
            EPI_LON:item.longitude,
            EPI_DEPTH:parseFloat(item.depth),
            AUTO_FLAG:(item.type==="automatic")?"(AUTO)":"M",
            EQ_TYPE:"M",
            M:item.magnitude,
            LOCATION_C:item.location
        });
    }
    return {shuju:shuju_array};
}

function history_onfail(num_errorcode){}

//根据URL判断该URL返回的是否为地震历史数据
function is_history_data(url){return url==="https://api.wolfx.jp/cenc_eqlist.json";}


//=========测站数据获取函数=============

//返回测站URL数量
function station_count(){return 2;}

//指定测站URL
function station_url(){return ["wss://seis.wolfx.jp/CQ_BEB_00","wss://seis.wolfx.jp/NM_EEDS_00"];}

//指定测站连接方式
function station_method(){return ["websocket","websocket"];}

//指定测站header（websocket以外）
function station_header(){return [{/*"Accept":"application/json"*/},{}];}

//指定测站postdata（post时）
function station_postdata(){return ["",""];}

const _station_config=[
                         {name:"重庆北碚",latitude:29.8108,longitude:106.3945,height:300},
                         {name:"内蒙古鄂尔多斯",latitude:39.5946,longitude:109.7899,height:1180}
                     ];

//收到数据后转换为下列格式
//{data:[{name:"AAA",//测站名或位置等信息
//        longitude:0.0,//测站经度
//        latitude:0.0,//测站纬度
//        height:0.0,//测站高度（米）
//        pga:0.0,//gal
//        pgv:0.0,//cm/s
//        pgd:0.0,//cm
//        intensity:0.0,//烈度
//        update:1234567890000//测站数据时间戳（毫秒），注意时区问题
//       },...]}
function station_onsuccess(num_index,str_response){
    var original=JSON.parse(str_response);
    return {data:[{name:_station_config[num_index].name,//测站名或位置等信息
                    longitude:_station_config[num_index].longitude,//测站经度
                    latitude:_station_config[num_index].latitude,//测站纬度
                    height:_station_config[num_index].height,//测站高度（米）
                    pga:original.PGA,//gal
                    pgv:original.PGV,//cm/s
                    pgd:original.PGD,//cm
                    intensity:original.Intensity,//烈度
                    update:fmt_to_msts(original.update_at+" UTC+8")//测站数据时间戳（毫秒），注意时区问题
                }]};
}

//错误处理
function station_onfail(num_errorcode){}

//判断URL是否是测站数据的URL（websocket以外）
function is_station_data(url){return url==="";}


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


//========自定义烈度，横波传播距离（以下函数若不使用请注释或删除）========

//magnitude:数值型，震级
//depth:数值型，深度（公里）
//返回值：数值型，震源烈度
//function calcMaxInt(magnitude,depth){
//    return 0;
//}

//magnitude:数值型，震级
//depth:数值型，深度（公里）
//epicenterLng:数值型，震源经度
//epicenterLat:数值型，震源纬度
//atLng:数值型，某个地点的经度
//atLat:数值型，某个地点的纬度
//返回值：数值型，某个地点的烈度
//function getIntensityAt(magnitude,depth,epicenterLng,epicenterLat,atLng,atLat){
//    return 0;
//}

//magnitude:数值型，震级
//depth:数值型，深度（公里）
//返回值：数值型，横波传播的最大距离（地表弧线距离，公里）
//function maxDistanceSWaveSpread(magnitude,depth){
//    return 0;
//}
