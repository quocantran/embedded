
#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

// Trang HTML chính - lưu trong Flash để tiết kiệm RAM
const char WEB_PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartFarm - Bảng điều khiển tưới thông minh</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=Be+Vietnam+Pro:wght@400;500;600;700;800&display=swap');

:root{
  --bg-1:#f6f9ef;
  --bg-2:#ebf3df;
  --ink:#1f2d1c;
  --muted:#5f6f56;
  --card:#ffffffd9;
  --card-border:#d8e5c8;
  --primary:#2f8f4e;
  --primary-2:#1f6d3b;
  --warning:#d48a15;
  --danger:#d54545;
  --info:#2378b8;
  --shadow:0 16px 40px rgba(38, 76, 45, 0.12);
  --radius:16px;
}

*{margin:0;padding:0;box-sizing:border-box}

body{
  font-family:'Be Vietnam Pro','Segoe UI',sans-serif;
  color:var(--ink);
  min-height:100vh;
  background:
    radial-gradient(1000px 500px at -10% -20%, #cce7b8 0%, transparent 60%),
    radial-gradient(800px 400px at 110% 0%, #b8dfc3 0%, transparent 60%),
    linear-gradient(165deg, var(--bg-1), var(--bg-2));
}

.header{
  position:relative;
  padding:24px 18px 18px;
  text-align:center;
  background:linear-gradient(130deg,#1f6d3b,#34985a);
  color:#fff;
  box-shadow:0 10px 24px rgba(31,109,59,.3);
  overflow:hidden;
}

.header:before,
.header:after{
  content:'';
  position:absolute;
  width:180px;
  height:180px;
  border-radius:50%;
  background:rgba(255,255,255,.09);
}

.header:before{top:-80px;left:-40px}
.header:after{bottom:-95px;right:-25px}

.header h1{
  position:relative;
  font-size:1.5em;
  font-weight:800;
  letter-spacing:.2px;
}

.header p{
  position:relative;
  margin-top:6px;
  font-size:.9em;
  opacity:.95;
}

.container{
  max-width:960px;
  margin:0 auto;
  padding:16px;
}

.card{
  background:var(--card);
  border:1px solid var(--card-border);
  border-radius:var(--radius);
  padding:16px;
  margin-bottom:14px;
  box-shadow:var(--shadow);
  backdrop-filter:blur(4px);
  animation:riseIn .45s ease both;
}

.card h2{
  font-size:1.05em;
  margin-bottom:12px;
  padding-bottom:8px;
  border-bottom:1px dashed #c8d8b8;
  display:flex;
  align-items:center;
  gap:8px;
}

.card h2 .icon{font-size:1.15em}

.desc{
  font-size:.78em;
  color:var(--muted);
  margin-top:8px;
  line-height:1.45;
}

.grid2{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}
.grid4{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px}

.stat{
  background:#f5fbe9;
  border:1px solid #d7e9c2;
  border-radius:12px;
  padding:10px;
  text-align:center;
}

.stat .val{font-size:1.35em;font-weight:800;color:var(--primary-2)}
.stat .lbl{font-size:.74em;color:var(--muted);margin-top:2px}
.stat.warn .val{color:var(--warning)}
.stat.danger .val{color:var(--danger)}
.stat.pump-on .val{color:var(--info);animation:pulse 1.1s infinite}

label{display:block;font-size:.82em;color:var(--muted);margin-bottom:5px;font-weight:600}

input[type=number],input[type=text],input[type=password],select{
  width:100%;
  padding:10px 11px;
  background:#fff;
  border:1px solid #c9dcb6;
  color:var(--ink);
  border-radius:10px;
  font-size:.9em;
  font-family:inherit;
}

input[type=number]:focus,input[type=text]:focus,input[type=password]:focus,select:focus{
  outline:none;
  border-color:var(--primary);
  box-shadow:0 0 0 3px rgba(47,143,78,.15);
}

.btn{
  display:inline-flex;
  justify-content:center;
  align-items:center;
  gap:6px;
  width:100%;
  border:none;
  border-radius:12px;
  cursor:pointer;
  padding:11px 16px;
  font-size:.9em;
  font-weight:700;
  font-family:inherit;
  transition:transform .18s ease, box-shadow .18s ease, filter .18s ease;
}

.btn:hover{transform:translateY(-1px);filter:saturate(1.06)}
.btn:active{transform:translateY(0)}

.btn-green{background:linear-gradient(145deg,#2f8f4e,#287b43);color:#fff;box-shadow:0 8px 18px rgba(47,143,78,.3)}
.btn-blue{background:linear-gradient(145deg,#2b87c5,#2372a8);color:#fff;box-shadow:0 8px 18px rgba(35,120,184,.28)}
.btn-orange{background:linear-gradient(145deg,#d9952f,#c48118);color:#fff;box-shadow:0 8px 18px rgba(212,138,21,.25)}

.mode-btns{display:flex;gap:8px}
.mode-btn{flex:1;padding:12px 8px;font-size:.82em}
.mode-btn.active{box-shadow:0 0 0 3px rgba(255,255,255,.85),0 0 0 6px rgba(47,143,78,.35)}

.sch-item{
  background:#f6fce9;
  border:1px solid #d4e6be;
  border-radius:12px;
  padding:10px;
  margin-bottom:8px;
}

.sch-head{font-size:.82em;font-weight:700;color:#355b32;margin-bottom:8px}
.sch-row{display:flex;gap:8px;align-items:flex-end;flex-wrap:wrap}
.sch-row input,.sch-row select{flex:1;min-width:50px}
.sch-field{min-width:86px;flex:1}
.sch-field label{font-size:.72em;margin-bottom:4px}
.sch-colon{font-weight:700;color:#4a5c42;padding-bottom:9px}
.sch-toggle{display:flex;align-items:center;gap:6px;padding-bottom:8px}
.sch-toggle span{font-size:.72em;color:#4a5c42;font-weight:700}
.days{display:flex;gap:4px;margin-top:8px;flex-wrap:wrap}
.days label{display:flex;align-items:center;gap:3px;font-size:.74em;cursor:pointer;color:#4a5c42;margin:0}
.days input[type=checkbox]{accent-color:var(--primary)}

.toast{
  position:fixed;
  top:16px;
  right:16px;
  padding:12px 16px;
  border-radius:12px;
  color:#fff;
  font-size:.85em;
  font-weight:600;
  opacity:0;
  transform:translateY(-6px);
  transition:opacity .25s ease, transform .25s ease;
  z-index:999;
  max-width:280px;
  box-shadow:0 12px 26px rgba(0,0,0,.2);
}

.toast.show{opacity:1;transform:translateY(0)}
.toast.ok{background:linear-gradient(145deg,#2f8f4e,#287b43)}
.toast.err{background:linear-gradient(145deg,#d55a5a,#bc4141)}

.switch{position:relative;width:46px;height:24px}
.switch input{display:none}

.slider{
  position:absolute;
  inset:0;
  background:#9eb090;
  border-radius:14px;
  cursor:pointer;
  transition:.3s;
}

.slider:before{
  content:'';
  position:absolute;
  width:18px;
  height:18px;
  left:3px;
  bottom:3px;
  background:#fff;
  border-radius:50%;
  transition:.3s;
}

.switch input:checked+.slider{background:var(--primary)}
.switch input:checked+.slider:before{transform:translateX(22px)}

@keyframes pulse{0%,100%{opacity:1}50%{opacity:.55}}
@keyframes riseIn{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:translateY(0)}}

@media (max-width:760px){
  .container{padding:12px}
  .grid4{grid-template-columns:repeat(2,minmax(0,1fr))}
}

@media (max-width:520px){
  .header h1{font-size:1.2em}
  .header p{font-size:.82em}
  .grid2{grid-template-columns:1fr}
}
</style>
</head>
<body>
<div class="header">
  <h1>🌱 Hệ thống tưới thông minh</h1>
  <p>Bảng điều khiển hệ thống tưới tiêu thông minh ESP32</p>
</div>
<div class="container">

<!-- TRANG THAI -->
<div class="card" id="status-card">
  <h2><span class="icon">📊</span> Trạng thái hiện tại</h2>
  <div class="grid4">
    <div class="stat"><div class="val" id="s-temp">--</div><div class="lbl">Nhiệt độ °C</div></div>
    <div class="stat"><div class="val" id="s-hum">--</div><div class="lbl">Độ ẩm không khí %</div></div>
    <div class="stat" id="s-soil-box"><div class="val" id="s-soil">--</div><div class="lbl">Độ ẩm đất %</div></div>
    <div class="stat" id="s-pump-box"><div class="val" id="s-pump">TẮT</div><div class="lbl">Máy bơm</div></div>
  </div>
  <div class="grid2" style="margin-top:10px">
    <div class="stat"><div class="val" id="s-mode" style="font-size:1em">--</div><div class="lbl">Chế độ</div></div>
    <div class="stat"><div class="val" id="s-budget" style="font-size:1em">--</div><div class="lbl">Ngân sách còn lại</div></div>
  </div>
</div>

<!-- CHE DO -->
<div class="card">
  <h2><span class="icon">⚙️</span> Chế độ hoạt động</h2>
  <div class="mode-btns">
    <button class="btn btn-green mode-btn" onclick="setMode(0)" id="btn-auto">Tự động</button>
    <button class="btn btn-orange mode-btn" onclick="setMode(1)" id="btn-manual">Thủ công</button>
    <button class="btn btn-blue mode-btn" onclick="setMode(2)" id="btn-sched">Theo lịch</button>
  </div>
  <p class="desc">⚠ Web chỉ gửi yêu cầu đổi chế độ. State machine sẽ quyết định hành động thực tế.</p>
</div>

<!-- NGUONG -->
<div class="card">
  <h2><span class="icon">🎛️</span> Cấu hình ngưỡng</h2>
  <div class="grid2">
    <div><label>Ngưỡng thấp (%)</label>
      <input type="number" id="c-low" min="5" max="90" value="30"></div>
    <div><label>Ngưỡng cao (%)</label>
      <input type="number" id="c-high" min="10" max="95" value="70"></div>
    <div><label>Ngưỡng nguy hiểm (%)</label>
      <input type="number" id="c-danger" min="1" max="50" value="15"></div>
    <div><label>Bơm tối đa (phút)</label>
      <input type="number" id="c-maxpump" min="1" max="30" value="5"></div>
    <div><label>Ngân sách/ngày (giây)</label>
      <input type="number" id="c-budget" min="60" max="3600" value="600"></div>
    <div><label>Thời gian nghỉ (phút)</label>
      <input type="number" id="c-cool" min="5" max="120" value="30"></div>
    <div><label>Thời gian mỗi xung (giây)</label>
      <input type="number" id="c-pulsesec" min="5" max="120" step="1" value="30"></div>
    <div><label>Nghỉ giữa các xung (giây)</label>
      <input type="number" id="c-checkdelaysec" min="1" max="120" step="1" value="5"></div>
    <div><label>Thoát MANUAL khi đất nguy hiểm quá (giây)</label>
      <input type="number" id="c-mantimeout" min="1" max="1800" step="1" value="60"></div>
  </div>
  <button class="btn btn-green" style="margin-top:12px" onclick="saveConfig()">
    💾 Lưu cấu hình</button>
</div>

<!-- LICH TUOI -->
<div class="card">
  <h2><span class="icon">📅</span> Lịch tưới (tối đa 4)</h2>
  <p class="desc">Mỗi lịch gồm giờ bắt đầu, phút bắt đầu và thời lượng tưới (giây). Chọn các ngày áp dụng rồi bấm lưu lịch tưới.</p>
  <p class="desc">Thời lượng mỗi lịch phải là bội số của thời gian mỗi xung.</p>
  <p class="desc">Lưu ý: Lịch kích hoạt theo giờ + phút (không có giây bắt đầu). Ô giây là số giây bơm chạy cho mỗi lịch.</p>
  <div id="schedules"></div>
  <button class="btn btn-blue" style="margin-top:10px" onclick="saveSchedules()">
    💾 Lưu lịch tưới</button>
</div>

<!-- HIEU CHUAN -->
<div class="card">
  <h2><span class="icon">🔧</span> Hiệu chuẩn cảm biến đất</h2>
  <div class="grid2">
    <div><label>ADC đất khô</label>
      <input type="number" id="cal-dry" min="0" max="4095" value="4095"></div>
    <div><label>ADC đất ướt</label>
      <input type="number" id="cal-wet" min="0" max="4095" value="1500"></div>
  </div>
  <p class="desc">Đặt cảm biến vào đất khô để đọc ADC khô. Nhúng vào nước để đọc ADC ướt.</p>
  <button class="btn btn-orange" style="margin-top:10px" onclick="saveCalibrate()">
    🔧 Lưu hiệu chuẩn</button>
</div>

</div><!-- container -->

<div class="toast" id="toast"></div>

<script>
const DAYS=['CN','T2','T3','T4','T5','T6','T7'];
let lastScheduleSig='';
let currentPulseSec=30;
let latestSchedules=[];

function hasConfiguredSchedule(schedules){
  if(!Array.isArray(schedules)) return false;
  return schedules.some(s=>
    s && Number(s.enabled)===1 &&
    Number(s.duration)>0 &&
    (Number(s.days)&0x7F)!==0
  );
}

function isUserEditingForm(){
  const el=document.activeElement;
  return !!(el && el.tagName && (el.tagName==='INPUT' || el.tagName==='SELECT' || el.tagName==='TEXTAREA'));
}

function updateStatusWidgets(d){
  document.getElementById('s-temp').textContent=d.temp!==null?d.temp.toFixed(1):'--';
  document.getElementById('s-hum').textContent=d.hum!==null?d.hum.toFixed(0):'--';
  document.getElementById('s-soil').textContent=d.soil!==null?d.soil:'--';
  const pb=document.getElementById('s-pump-box');
  const pv=document.getElementById('s-pump');
  if(d.pump){pv.textContent='BẬT';pb.className='stat pump-on';}
  else{pv.textContent='TẮT';pb.className='stat';}
  const sb=document.getElementById('s-soil-box');
  if(d.soil!==null&&d.soil<d.danger)sb.className='stat danger';
  else if(d.soil!==null&&d.soil<d.low)sb.className='stat warn';
  else sb.className='stat';
  const modes=['Tự động','Thủ công','Theo lịch'];
  document.getElementById('s-mode').textContent=modes[d.mode]||'?';
  document.getElementById('s-budget').textContent=d.budgetLeft+' giây';
  ['btn-auto','btn-manual','btn-sched'].forEach((id,i)=>{
    document.getElementById(id).classList.toggle('active',i===d.mode);
  });
}

function updateEditableFields(d){
  document.getElementById('c-low').value=d.low;
  document.getElementById('c-high').value=d.high;
  document.getElementById('c-danger').value=d.danger;
  document.getElementById('c-maxpump').value=d.maxPump;
  document.getElementById('c-budget').value=d.budget;
  document.getElementById('c-cool').value=d.cooldown;
  currentPulseSec=(typeof d.pulseSec==='number'&&d.pulseSec>=5&&d.pulseSec<=120)?d.pulseSec:30;
  document.getElementById('c-pulsesec').value=currentPulseSec;
  const checkDelaySec=(typeof d.checkDelaySec==='number'&&d.checkDelaySec>=1&&d.checkDelaySec<=120)?d.checkDelaySec:5;
  document.getElementById('c-checkdelaysec').value=checkDelaySec;
  const manTimeoutSec=(typeof d.manTimeoutSec==='number')
    ?d.manTimeoutSec
    :((typeof d.manTimeout==='number')?(d.manTimeout*60):60);
  document.getElementById('c-mantimeout').value=manTimeoutSec;
  document.getElementById('cal-dry').value=d.calDry;
  document.getElementById('cal-wet').value=d.calWet;
  latestSchedules=Array.isArray(d.schedules)?d.schedules:[];

  const scheduleSig=JSON.stringify({pulseSec:currentPulseSec,schedules:latestSchedules});
  if(scheduleSig!==lastScheduleSig){
    renderSchedules(latestSchedules);
    lastScheduleSig=scheduleSig;
  }
}

function toast(msg,ok){
  const t=document.getElementById('toast');
  t.textContent=msg;t.className='toast show '+(ok?'ok':'err');
  setTimeout(()=>t.className='toast',3000);
}

function fetchStatus(){
  fetch('/api/status').then(r=>r.json()).then(d=>{
    updateStatusWidgets(d);
    if(!isUserEditingForm()){
      updateEditableFields(d);
    }
  }).catch(e=>console.error('Lỗi:',e));
}

function renderSchedules(scheds){
  const c=document.getElementById('schedules');
  const pulseSec=(typeof currentPulseSec==='number'&&currentPulseSec>=5&&currentPulseSec<=120)?currentPulseSec:30;
  const minDuration=Math.max(10,pulseSec);
  let html='';
  for(let i=0;i<4;i++){
    const s=scheds&&scheds[i]?scheds[i]:{enabled:0,hour:6,minute:0,days:127,duration:pulseSec*2};
    const rawDuration=(typeof s.duration==='number'&&s.duration>=minDuration&&s.duration<=600)?s.duration:(pulseSec*2);
    let duration=Math.max(minDuration,Math.min(600,rawDuration));
    if(duration%pulseSec!==0){
      duration=Math.ceil(duration/pulseSec)*pulseSec;
      if(duration>600)duration=Math.floor(600/pulseSec)*pulseSec;
      if(duration<minDuration)duration=minDuration;
    }
    html+='<div class="sch-item">';
    html+='<div class="sch-head">Lịch '+(i+1)+'</div>';
    html+='<div class="sch-row">';
    html+='<div class="sch-toggle"><label class="switch"><input type="checkbox" id="sch-en-'+i+'" '+(s.enabled?'checked':'')+
      '><span class="slider"></span></label><span>Bật</span></div>';
    html+='<div class="sch-field"><label for="sch-h-'+i+'">Giờ (0-23)</label>';
    html+='<input type="number" id="sch-h-'+i+'" min="0" max="23" value="'+s.hour+'"></div>';
    html+='<div class="sch-colon">:</div>';
    html+='<div class="sch-field"><label for="sch-m-'+i+'">Phút (0-59)</label>';
    html+='<input type="number" id="sch-m-'+i+'" min="0" max="59" value="'+s.minute+'"></div>';
    html+='<div class="sch-field"><label for="sch-dur-'+i+'">Thời lượng (giây)</label>';
    html+='<input type="number" id="sch-dur-'+i+'" min="'+minDuration+'" max="600" step="'+pulseSec+'" value="'+duration+'"></div>';
    html+='</div>';
    html+='<div class="days">';
    for(let d=0;d<7;d++){
      const chk=(s.days>>d)&1;
      html+='<label><input type="checkbox" id="sch-d-'+i+'-'+d+'" '+(chk?'checked':'')+'>'+DAYS[d]+'</label>';
    }
    html+='</div></div>';
  }
  c.innerHTML=html;
}

function setMode(m){
  if(m===2 && !hasConfiguredSchedule(latestSchedules)){
    toast('Hay cau hinh lich tuoi truoc khi vao mode SCHEDULE',false);
    return;
  }

  fetch('/api/mode',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({mode:m})}).then(r=>r.json()).then(d=>{
    if(d.ok)toast('Đã gửi yêu cầu đổi chế độ',true);
    else toast('Lỗi: '+d.msg,false);
    setTimeout(fetchStatus,500);
  }).catch(()=>toast('Lỗi kết nối',false));
}

function saveConfig(){
  const manTimeoutSec=+document.getElementById('c-mantimeout').value;
  const pulseSec=+document.getElementById('c-pulsesec').value;
  const checkDelaySec=+document.getElementById('c-checkdelaysec').value;
  if(manTimeoutSec<1||manTimeoutSec>1800){
    toast('Thoát MANUAL phải từ 1 đến 1800 giây',false);
    return;
  }
  if(pulseSec<5||pulseSec>120){
    toast('Thời gian mỗi xung phải từ 5 đến 120 giây',false);
    return;
  }
  if(checkDelaySec<1||checkDelaySec>120){
    toast('Nghỉ giữa các xung phải từ 1 đến 120 giây',false);
    return;
  }

  const data={
    low:+document.getElementById('c-low').value,
    high:+document.getElementById('c-high').value,
    danger:+document.getElementById('c-danger').value,
    maxPump:+document.getElementById('c-maxpump').value,
    budget:+document.getElementById('c-budget').value,
    cooldown:+document.getElementById('c-cool').value,
    manTimeoutSec:manTimeoutSec,
    pulseSec:pulseSec,
    checkDelaySec:checkDelaySec
  };
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(data)}).then(r=>r.json()).then(d=>{
    if(d.ok)toast('Lưu cấu hình thành công!',true);
    else toast('Lỗi: '+d.msg,false);
  }).catch(()=>toast('Lỗi kết nối',false));
}

function saveSchedules(){
  const pulseSec=+document.getElementById('c-pulsesec').value||currentPulseSec;
  const minDuration=Math.max(10,pulseSec);
  if(pulseSec<5||pulseSec>120){
    toast('Thời gian mỗi xung phải từ 5 đến 120 giây',false);
    return;
  }

  const scheds=[];
  for(let i=0;i<4;i++){
    const enabled=document.getElementById('sch-en-'+i).checked?1:0;
    const hour=+document.getElementById('sch-h-'+i).value;
    const minute=+document.getElementById('sch-m-'+i).value;
    const duration=+document.getElementById('sch-dur-'+i).value;

    if(hour<0||hour>23||minute<0||minute>59){
      toast('Lịch '+(i+1)+': Giờ/phút không hợp lệ',false);
      return;
    }

    let days=0;
    for(let d=0;d<7;d++){
      if(document.getElementById('sch-d-'+i+'-'+d).checked)days|=(1<<d);
    }

    if(enabled&&days===0){
      toast('Lịch '+(i+1)+': Hãy chọn ít nhất 1 ngày',false);
      return;
    }

    if(duration<minDuration||duration>600){
      toast('Lịch '+(i+1)+': Thời lượng phải từ '+minDuration+' đến 600 giây',false);
      return;
    }
    if(duration%pulseSec!==0){
      toast('Lịch '+(i+1)+': Thời lượng phải là bội số của '+pulseSec+' giây',false);
      return;
    }

    scheds.push({
      enabled:enabled,
      hour:hour,
      minute:minute,
      days:days,
      duration:duration
    });
  }
  fetch('/api/schedule',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({schedules:scheds})}).then(r=>r.json()).then(d=>{
    if(d.ok){
      latestSchedules=scheds;
      lastScheduleSig=JSON.stringify({pulseSec:currentPulseSec,schedules:latestSchedules});
      renderSchedules(latestSchedules);
      toast('Lưu lịch tưới thành công!',true);
      setTimeout(fetchStatus,300);
    }
    else toast('Lỗi: '+d.msg,false);
  }).catch(()=>toast('Lỗi kết nối',false));
}

function saveCalibrate(){
  const data={dry:+document.getElementById('cal-dry').value,
    wet:+document.getElementById('cal-wet').value};
  fetch('/api/calibrate',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(data)}).then(r=>r.json()).then(d=>{
    if(d.ok)toast('Hiệu chuẩn thành công!',true);
    else toast('Lỗi: '+d.msg,false);
  }).catch(()=>toast('Lỗi kết nối',false));
}

fetchStatus();
setInterval(fetchStatus,5000);
</script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
