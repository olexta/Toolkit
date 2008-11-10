USE [master]
GO
/****** Object:  Database [%DB_NAME%]    Script Date: 08/22/2007 15:43:16 ******/
CREATE DATABASE [%DB_NAME%] ON  PRIMARY 
( NAME = N'%DB_NAME%', FILENAME = N'%DB_PATH%\%DB_NAME%.MDF' , SIZE = 5120KB , MAXSIZE = UNLIMITED, FILEGROWTH = 10%)
 LOG ON 
( NAME = N'%DB_NAME%_Log', FILENAME = N'%DB_PATH%\%DB_NAME%.LDF' , SIZE = 1024KB , MAXSIZE = UNLIMITED , FILEGROWTH = 10%)
GO
EXEC dbo.sp_dbcmptlevel @dbname=N'%DB_NAME%', @new_cmptlevel=80
GO
IF (1 = FULLTEXTSERVICEPROPERTY('IsFullTextInstalled'))
begin
EXEC [%DB_NAME%].[dbo].[sp_fulltext_database] @action = 'disable'
end
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ARITHABORT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_CLOSE ON 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_CREATE_STATISTICS ON 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_SHRINK ON 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [%DB_NAME%] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [%DB_NAME%] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [%DB_NAME%] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [%DB_NAME%] SET RECOVERY SIMPLE 
GO
ALTER DATABASE [%DB_NAME%] SET  MULTI_USER 
GO
USE [%DB_NAME%]
GO
/****** Object:  Table [dbo].[_objects]    Script Date: 08/22/2007 15:25:23 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[_objects](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectName] [nvarchar](255) NOT NULL,
	[ObjectType] [nvarchar](255) NOT NULL,
	[TimeStamp] [datetime] NULL,
 CONSTRAINT [PK_Objects] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
) ON [PRIMARY]
) ON [PRIMARY]
GO
CREATE NONCLUSTERED INDEX [IX_Objects_ObjectType] ON [dbo].[_objects] 
(
	[ObjectType] ASC
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[_images]    Script Date: 08/22/2007 15:25:17 ******/
GO
CREATE TABLE [dbo].[_images](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectID] [int] NOT NULL,
	[Name] [nvarchar](50) NOT NULL,
	[Value] [image] NULL,
 CONSTRAINT [PK__images] PRIMARY KEY CLUSTERED 
(
	[ObjectID] ASC,
	[Name] ASC
) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
/****** Object:  Table [dbo].[_links]    Script Date: 08/22/2007 15:25:20 ******/
GO
CREATE TABLE [dbo].[_links](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[Parent] [int] NOT NULL,
	[Child] [int] NOT NULL,
 CONSTRAINT [PK__links] PRIMARY KEY CLUSTERED 
(
	[Parent] ASC,
	[Child] ASC
) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[_properties]    Script Date: 08/22/2007 15:25:26 ******/
CREATE TABLE [dbo].[_properties](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectID] [int] NOT NULL,
	[Name] [nvarchar](50) NOT NULL,
	[Value] [sql_variant] NULL,
 CONSTRAINT [PK_Properties] PRIMARY KEY CLUSTERED 
(
	[ObjectID] ASC,
	[Name] ASC
) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Trigger [objects]    Script Date: 08/22/2007 15:25:28 ******/
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[objects] ON [dbo].[_objects] FOR INSERT, UPDATE
AS
    UPDATE [_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN (SELECT [ID] FROM [inserted])
GO
/****** Object:  Trigger [images_objects]    Script Date: 08/22/2007 15:25:27 ******/
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[images_objects] ON [dbo].[_images] FOR INSERT, UPDATE, DELETE
AS
    -- check for unique property names
    IF ( EXISTS(SELECT *
        FROM (
            SELECT [ObjectID], [Name] FROM [_properties]
            UNION
            SELECT [ObjectID], [Name] FROM [_images]) p
            GROUP BY
            [ObjectID], [Name]
            HAVING
            COUNT(*) > 1) ) BEGIN
        RAISERROR ('There is property unique naming conflict!', 11, 1)
        ROLLBACK TRANSACTION
    END
    -- update object timestamp
    UPDATE [_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN ((SELECT [ObjectID] FROM [deleted]) UNION (SELECT [ObjectID] FROM [inserted]))
GO
/****** Object:  Trigger [links_objects]    Script Date: 08/22/2007 15:25:28 ******/
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[links_objects] ON [dbo].[_links] FOR INSERT, UPDATE, DELETE
AS
  -- update object timestamp
  UPDATE [_objects] SET [TimeStamp] = GETDATE()
  WHERE [ID] IN (SELECT [Parent] FROM [deleted]
                 UNION
                 SELECT [Parent] FROM [inserted])
GO

/****** Object:  Trigger [properties_objects]    Script Date: 08/22/2007 15:25:28 ******/
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[properties_objects] ON [dbo].[_properties] FOR INSERT, UPDATE, DELETE
AS
    -- check for unique property names
    IF ( EXISTS(SELECT *
        FROM ( SELECT [ObjectID], [Name] FROM [_properties]
            UNION
            SELECT [ObjectID], [Name] FROM [_images]) p
            GROUP BY
            [ObjectID], [Name]
            HAVING
            COUNT(*) > 1) ) BEGIN
        RAISERROR ('There is property unique naming conflict!', 11, 1)
        ROLLBACK TRANSACTION
    END

    -- update object timestamp
    UPDATE [_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN ((SELECT [ObjectID] FROM [deleted]) UNION (SELECT [ObjectID] FROM [inserted]))
GO
/****** Object:  ForeignKey [FK_images_objects]    Script Date: 08/22/2007 15:25:17 ******/
ALTER TABLE [dbo].[_images]  WITH NOCHECK ADD  CONSTRAINT [FK_images_objects] FOREIGN KEY([ObjectID])
REFERENCES [dbo].[_objects] ([ID])
ON DELETE CASCADE
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_images] CHECK CONSTRAINT [FK_images_objects]
GO
/****** Object:  ForeignKey [FK_links_Parent]    Script Date: 08/22/2007 15:25:20 ******/
ALTER TABLE [dbo].[_links]  WITH NOCHECK ADD  CONSTRAINT [FK_links_Parent] FOREIGN KEY([Parent])
REFERENCES [dbo].[_objects] ([ID])
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_links] CHECK CONSTRAINT [FK_links_Parent]
GO
/****** Object:  ForeignKey [FK_links_Child]    Script Date: 08/22/2007 15:25:20 ******/
ALTER TABLE [dbo].[_links]  WITH NOCHECK ADD  CONSTRAINT [FK_links_Child] FOREIGN KEY([Child])
REFERENCES [dbo].[_objects] ([ID])
ON DELETE CASCADE
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_links] CHECK CONSTRAINT [FK_links_Child]
GO
/****** Object:  ForeignKey [FK_properties_objects]    Script Date: 08/22/2007 15:25:26 ******/
ALTER TABLE [dbo].[_properties]  WITH NOCHECK ADD  CONSTRAINT [FK_properties_objects] FOREIGN KEY([ObjectID])
REFERENCES [dbo].[_objects] ([ID])
ON DELETE CASCADE
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_properties] CHECK CONSTRAINT [FK_properties_objects]
GO