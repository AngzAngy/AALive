package cn.kuwo.mod.mobilead.audioad;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.model.GlideUrl;
import com.bumptech.glide.load.model.LazyHeaders;
import com.bumptech.glide.request.target.SimpleTarget;
import com.bumptech.glide.request.transition.Transition;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import cn.kuwo.base.bean.Music;
import cn.kuwo.base.http.HttpResult;
import cn.kuwo.base.http.HttpSession;
import cn.kuwo.base.image.BitmapUtils;
import cn.kuwo.base.log.LogMgr;
import cn.kuwo.base.util.KwDate;
import cn.kuwo.base.util.KwThreadPool;
import cn.kuwo.base.util.NetworkStateUtil;
import cn.kuwo.base.util.UrlManagerUtils;
import cn.kuwo.kwmusiccar.App;
import cn.kuwo.kwmusiccar.ad.AdJsonParse;
import cn.kuwo.kwmusiccar.ad.entity.AdEntity;
import cn.kuwo.kwmusiccar.ad.entity.TencentAdLyric;
import cn.kuwo.mod.lyric.ILyrics;
import cn.kuwo.mod.lyric.LyricsDefine;

import static cn.kuwo.mod.lyric.LyricsSendNotice.sendSyncNotice_HeadPicFinished;
import static cn.kuwo.mod.lyric.LyricsSendNotice.sendSyncNotice_LyricFinished;

public class TencentAudioAdMgr {
    private static final String TAG = "TencentAudioAdMgr";
//    private static final int SONG_COVER_W = 320;
//    private static final int SONG_COVER_H = 320;
    private List<AdEntity> adEntityList;
    private AdEntity adEntity;
    private Music adMusic;
    private boolean isPlaying;

    public Music getAdMusic(){
        return adMusic;
    }

    public void requestRes(){
        adEntityList = null;
        isPlaying = false;
        if(NetworkStateUtil.isAvaliable()){
            KwThreadPool.runThread(KwThreadPool.JobType.NET, new Runnable() {
                @Override
                public void run() {
                    String requestJson = AdJsonParse.getRequestJson();
                    LogMgr.e(TAG, requestJson);
                    byte[] bytes = String.valueOf("Request=" + requestJson).getBytes();
                    HttpSession httpSession = new HttpSession();
                    HttpResult httpResult = httpSession.post(UrlManagerUtils.TENCENT_AD_BASH_URL, bytes);
                    if(httpResult != null && httpResult.data != null){
                        try {
                            parseResule(new String(httpResult.data));
                            newAdMusic();
                        }catch (Throwable throwable){
                            adEntityList = null;
                            relealseAdMusic();
                            throwable.printStackTrace();
                        }
                    }
                }
            });
        }
    }

    private void parseResule(String jsonStr) throws JSONException{
        JSONObject json = new JSONObject(jsonStr);

        //获取服务器时间 和本地 对比，服务器时间比本地时间大就用服务器时间
        long time = json.optLong("CurrentTime") * 1000L;
        KwDate today = new KwDate();
        if (today.getTime() < time && time > 0) {
            today = new KwDate(time);
        }

        JSONArray ad = json.optJSONArray("Ad");
        if (ad == null || ad.length() == 0) {
//            error(errorUrlMain, "JsonFailed");
            return;
        }

        adEntityList = AdJsonParse.initJsonAd(ad, null);

        for (AdEntity adEntity : adEntityList) {
            //广告到期
            KwDate end = new KwDate(adEntity.endTime);
            if (end.before(today)) {
                continue;
            }

            //广告没到 开始展示的时间
            KwDate start = new KwDate(adEntity.startTime);
            if (!start.before(today)) {
                continue;
            }
            //判空
            if (adEntity.media == null || adEntity.splashImg == null) {
                continue;
            }
        }
    }

    private void newAdMusic(){
        if(adEntityList != null && !adEntityList.isEmpty()){
            adEntity = adEntityList.get(0);
            if(adEntity != null && adEntity.media != null &&
                    !TextUtils.isEmpty(adEntity.media.staticResource)){
                adMusic = new Music();
                adMusic.filePath = adEntity.media.staticResource;
                adMusic.duration = adEntity.media.duration;
                adMusic.name = adEntity.adTitle;
                adMusic.artist = adEntity.advertiser;
            }
        }
    }

    private void relealseAdMusic(){
        adMusic = null;
        adEntity = null;
    }

    public void playStart(){
        isPlaying = true;
        //请求图片资源
        downloadHeadPic(adEntity, adMusic);
        //歌词
        downloadLyric(adEntity, adMusic);
    }

    public boolean playFail(){
        if(isPlaying){//当前广告播放失败，直接播放下一曲音乐
            relealseAdMusic();
            isPlaying = false;
            return true;
        }
        return false;
    }

    public boolean playStop(boolean end){
        if(isPlaying){//当前广告播放完成，直接播放下一曲音乐
            relealseAdMusic();
            isPlaying = false;
            return true;
        }
        return false;
    }

    private void downloadHeadPic(final AdEntity adEntity,final Music adMusic){
        if(adEntity!= null && adEntity.songCover!= null
                && !TextUtils.isEmpty(adEntity.songCover.staticResource)){
            GlideUrl glideUrl = new GlideUrl(adEntity.songCover.staticResource, new LazyHeaders.Builder()
                    .addHeader("Referer", "http://www.kuwo.cn")
                    .build());
            Glide.with(App.getInstance())
                    .asBitmap()
                    .load(glideUrl)
                    .into(new SimpleTarget<Bitmap>() {
                        @Override
                        public void onLoadStarted(@Nullable Drawable var1){
                            sendSyncNotice_HeadPicFinished(adMusic, LyricsDefine.DownloadStatus.BEGIN);
                        }

                        @Override
                        public void onLoadFailed(@Nullable Drawable var1){
                            sendSyncNotice_HeadPicFinished(null, LyricsDefine.DownloadStatus.FAILED, null);
                        }
                        @Override
                        public void onResourceReady(@NonNull Bitmap bitmap, @Nullable Transition<? super Bitmap> transition) {
                            if(!BitmapUtils.bitmapIsRecycled(bitmap)){
                                sendSyncNotice_HeadPicFinished(adMusic, LyricsDefine.DownloadStatus.SUCCESS, bitmap);
                            }else{
                                sendSyncNotice_HeadPicFinished(null, LyricsDefine.DownloadStatus.NONE, null);
                            }
                        }
                    });
        }
    }

    private void downloadLyric(final AdEntity adEntity,final Music adMusic){
        sendSyncNotice_LyricFinished(adMusic, LyricsDefine.DownloadStatus.BEGIN, false);
        if(adEntity!= null && adMusic != null) {
            TencentAdLyric tencentAdLyric = new TencentAdLyric();
            tencentAdLyric.setSongName(adEntity.adTitle);
            tencentAdLyric.setArtist(adEntity.advertiser);
            ArrayList<String> all = new ArrayList<>();
            all.add(adEntity.description);
            tencentAdLyric.setAllSentences(all);

            sendSyncNotice_LyricFinished(adMusic, LyricsDefine.DownloadStatus.SUCCESS, tencentAdLyric, tencentAdLyric, false);
        }else{
            sendSyncNotice_LyricFinished(adMusic, LyricsDefine.DownloadStatus.FAILED, false);
        }
    }
}
