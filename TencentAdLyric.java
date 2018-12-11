package cn.kuwo.kwmusiccar.ad.entity;


import java.util.List;

import cn.kuwo.mod.lyric.ILyrics;
import cn.kuwo.mod.lyric.LyricsDefine;

public class TencentAdLyric implements ILyrics {
    private String songName;
    private String artist;
    private List<String> allSentences;

    public void setAllSentences(List<String> sentences){
        allSentences = sentences;
    }

    @Override
    public List<String> getAllSentences() {
        return allSentences;
    }

    @Override
    public List<Integer> getSenStartTime() {
        return null;
    }

    @Override
    public List<List<LyricsDefine.OneWord>> getSenWordTime() {
        return null;
    }

    @Override
    public boolean getNowPlaying(long playTime, LyricsDefine.LyricsPlayInfo playInfo) {
        return false;
    }

    public void setSongName(String name){
        songName = name;
    }

    @Override
    public String getSongName() {
        return songName;
    }

    @Override
    public String getAlbum() {
        return null;
    }

    public void setArtist(String artist) {
        this.artist = artist;
    }

    @Override
    public String getArtist() {
        return artist;
    }

    @Override
    public String getBy() {
        return null;
    }

    @Override
    public void setOffset(long offset) {

    }

    @Override
    public long getOffset() {
        return 0;
    }

    @Override
    public LyricsDefine.LyricsType getType() {
        return LyricsDefine.LyricsType.TENCENTAD;
    }

    @Override
    public boolean isOriginal() {
        return false;
    }

    @Override
    public ILyrics getClipLyrics(int viewWidth) {
        return this;
    }
}
