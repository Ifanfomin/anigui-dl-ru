import sys
import json
import socks
import socket
from anicli_api.source import anilibria, animego, animevost, dreamcast, sameband, yummy_anime, yummy_anime_org


if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

def get_extractor(provider):
    match provider:
        case "anilibria":
            ex = anilibria.Extractor()
        case "animego":
            ex = animego.Extractor()
        case "animevost":
            ex = animevost.Extractor()
        case "dreamcast":
            ex = dreamcast.Extractor()
        case "sameband":
            ex = sameband.Extractor()
        case "yummy_anime":
            ex = yummy_anime.Extractor()
        case "yummy_anime_org":
            ex = yummy_anime_org.Extractor()

    return ex

def search_anime(provider, name):
    ex = get_extractor(provider)
    results = ex.search(name)
    if results:
        return [{"title": result.title} for result in results]
    else:
        return [{"title": "empty search result"}]

def get_episodes(provider, anime_name, anime_num):
    ex = get_extractor(provider)
    results = ex.search(anime_name)
    episodes = results[anime_num].get_anime().get_episodes()
    return [{"title": f"{i + 1}. {episode.title}"} for i, episode in enumerate(episodes)]

def get_sources(provider, anime_name, anime_num, episodes_nums):
    ex = get_extractor(provider)
    results = ex.search(anime_name)
    sources = results[anime_num].get_anime().get_episodes()[episodes_nums[0]].get_sources()
    output = []
    for i, source in enumerate(sources):
        if len(source.url.split("/")) > 2:
            output.append({"title": source.title, "url": source.url.split("/")[2]})
        else:
            output.append({"title": source.title, "url": ""})
    
    return output

def get_video_qualities(provider, anime_name, anime_num, episodes_nums, source_num):
    ex = get_extractor(provider)
    results = ex.search(anime_name)
    videos = results[anime_num].get_anime().get_episodes()[episodes_nums[0]].get_sources()[source_num].get_videos()

    return [{
        "title": f"{video.quality}",
        "url": video.url,
        "type": video.type
    } for video in videos]

def get_videos_urls(provider, anime_name, anime_num, episodes_nums, source_num):
    ex = get_extractor(provider)
    results = ex.search(anime_name)
    episodes = results[anime_num].get_anime().get_episodes()
    videos = []
    for num, episode in enumerate(episodes):
        if num in episodes_nums:
            videos.append(episode.get_sources()[source_num].get_videos())

    return [[{
        "title": f"{url.quality}",
        "url": url.url,
        "type": url.type
    } for url in video] for video in videos]

def main():
    n = 0

    if sys.argv[n + 1] == "proxy":
        socks.set_default_proxy(
            socks.SOCKS5,
            sys.argv[n + 2], # ip
            int(sys.argv[n + 3]), # port
            username=sys.argv[n + 4], # user
            password=sys.argv[n + 5] # pass
        )
        n = 5
        socket.socket = socks.socksocket

    if sys.argv[n + 1] == "search":
        provider = sys.argv[n + 2]
        anime_name = sys.argv[n + 3]
        print(json.dumps(search_anime(provider, anime_name), ensure_ascii=False))

    elif sys.argv[n + 1] == "episodes":
        provider = sys.argv[n + 2]
        anime_name = sys.argv[n + 3]
        anime_num = int(sys.argv[n + 4])
        print(json.dumps(get_episodes(provider, anime_name, anime_num), ensure_ascii=False))
    
    elif sys.argv[n + 1] == "sources":
        provider = sys.argv[n + 2]
        anime_name = sys.argv[n + 3]
        anime_num = int(sys.argv[n + 4])
        episodes_nums = [int(x) for x in sys.argv[n + 5].split()]
        print(json.dumps(get_sources(provider, anime_name, anime_num, episodes_nums), ensure_ascii=False))
    
    elif sys.argv[n + 1] == "video_qualities":
        provider = sys.argv[n + 2]
        anime_name = sys.argv[n + 3]
        anime_num = int(sys.argv[n + 4])
        episodes_nums = [int(x) for x in sys.argv[n + 5].split()]
        source_num = int(sys.argv[n + 6])
        print(json.dumps(get_video_qualities(provider, anime_name, anime_num, episodes_nums, source_num), ensure_ascii=False))

    elif sys.argv[n + 1] == "videos_urls":
        provider = sys.argv[n + 2]
        anime_name = sys.argv[n + 3]
        anime_num = int(sys.argv[n + 4])
        episodes_nums = [int(x) for x in sys.argv[n + 5].split()]
        source_num = int(sys.argv[n + 6])
        print(json.dumps(get_videos_urls(provider, anime_name, anime_num, episodes_nums, source_num), ensure_ascii=False))
    
if __name__ == "__main__":
    main()
